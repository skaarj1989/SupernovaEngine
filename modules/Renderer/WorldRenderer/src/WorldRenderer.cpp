#include "renderer/WorldRenderer.hpp"
#include "rhi/RenderDevice.hpp"

#include "renderer/CubemapConverter.hpp"

#include "FrameGraphResourceAccess.hpp"

#include "renderer/Vertex1p1n1st.hpp"
#include "renderer/MeshInstance.hpp"
#include "renderer/DecalInstance.hpp"
#include "renderer/Light.hpp"
#include "renderer/Grid.hpp"
#include "renderer/RenderSettings.hpp"
#include "renderer/WorldView.hpp"

#include "PerspectiveCamera.hpp"
#include "DebugDraw.hpp"

#include "renderer/ViewInfo.hpp"
#include "LightingSettings.hpp"
#include "LightingPassFeatures.hpp"

#include "BuildPropertyBuffer.hpp"

#include "fg/FrameGraph.hpp"
#include "fg/Blackboard.hpp"
#include "FrameGraphImport.hpp"
#include "renderer/FrameGraphBuffer.hpp"

#include "UploadFrameBlock.hpp"
#include "UploadCameraBlock.hpp"
#include "UploadTransforms.hpp"
#include "UploadSkins.hpp"
#include "UploadMaterialProperties.hpp"
#include "UploadLights.hpp"

#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/MaterialProperties.hpp"
#include "FrameGraphData/SceneColor.hpp"
#include "FrameGraphData/BRDF.hpp"
#include "FrameGraphData/SkyLight.hpp"
#include "FrameGraphData/UserData.hpp"

#include "RenderContext.hpp"
#include "ShaderCodeBuilder.hpp"

namespace gfx {

namespace {

constexpr auto kUseWeightedBlendedTechnique = false;
constexpr auto kTileSize = 16u;

void importSkyLight(FrameGraph &fg, FrameGraphBlackboard &blackboard,
                    const SkyLight &skyLight) {
  blackboard.add<SkyLightData>() = {
    .environment = importTexture(fg, "Skybox", skyLight.environment.get()),
    .diffuse = importTexture(fg, "DiffuseIBL", skyLight.diffuse.get()),
    .specular = importTexture(fg, "SpecularIBL", skyLight.specular.get()),
  };
}

[[nodiscard]] auto getFirstDirectionalLight(std::span<const Light *> lights) {
  const auto it = std::ranges::find_if(lights, [](const auto *light) {
    return light->type == LightType::Directional;
  });
  return it != lights.cend() ? *it : nullptr;
}

[[nodiscard]] auto
getVisibleRenderables(std::span<const Renderable> renderables,
                      const Frustum &frustum) {
  ZoneScopedN("GetVisibleRenderables");

  std::vector<const Renderable *> result;
  result.reserve(renderables.size());
  for (const auto &r : renderables) {
    if (frustum.testAABB(r.subMeshInstance.aabb)) result.emplace_back(&r);
  }
  return result;
}

// ---

// Key = Property buffer size (_PropertyBlock struct stride).
using PropertyGroups =
  std::unordered_map<std::size_t,
                     std::vector<std::pair<const PropertyLayout *,
                                           const std::vector<Property> *>>>;

struct PropertyBlock {
  std::vector<std::byte> buffer; // A blob ready to upload to GPU.
  PropertyGroupOffsets offsets;
};
[[nodiscard]] auto mergeProperties(const PropertyGroups &groups,
                                   std::size_t minOffsetAlignment) {
  ZoneScopedN("MergeProperties");

  PropertyBlock block;
  block.buffer.reserve(1000);
  for (const auto &[bufferSize, group] : groups) {
    block.offsets[bufferSize] = block.buffer.size();
    for (const auto &[layout, properties] : group) {
      auto rawProperties =
        buildPropertyBuffer(*layout, *properties, minOffsetAlignment);
      block.buffer.insert(block.buffer.cend(), rawProperties.cbegin(),
                          rawProperties.cend());
    }
  }
  return block;
}

struct RenderableStore {
  struct Config {
    std::size_t numTransforms;
    std::size_t numJoints;
  };
  explicit RenderableStore(const Config &config) {
    modelMatrices.reserve(config.numTransforms);
    joints.reserve(config.numJoints);
  }

  std::vector<glm::mat4> modelMatrices;
  Joints joints;
  PropertyGroups propertyGroups;
};

[[nodiscard]] auto
addMaterialInstance(RenderableStore &store,
                    const MaterialInstance &materialInstance) {
  std::optional<uint32_t> materialId;
  if (const auto &layout = materialInstance->getPropertyLayout();
      layout.stride > 0) {
    auto &group = store.propertyGroups[layout.stride];
    materialId = uint32_t(group.size());
    group.emplace_back(std::addressof(layout),
                       std::addressof(materialInstance.getProperties()));
  }
  return materialId;
}

[[nodiscard]] auto buildRenderables(RenderableStore &store,
                                    const auto &meshes) {
  ZoneScopedN("BuildRenderables");

  std::vector<Renderable> renderables;
  renderables.reserve(meshes.size());

  for (const auto *meshInstance : meshes) {
    if (!meshInstance) continue;

    const auto transformId = uint32_t(store.modelMatrices.size());
    store.modelMatrices.emplace_back(meshInstance->getModelMatrix());

    const auto skinOffset = meshInstance->hasSkin()
                              ? std::optional{uint32_t(store.joints.size())}
                              : std::nullopt;

    if (const auto &skin = meshInstance->getSkinMatrices(); !skin.empty())
      store.joints.insert(store.joints.cend(), skin.cbegin(), skin.cend());

    for (const SubMeshInstance &subMesh : meshInstance->each()) {
      if (!subMesh.visible || !subMesh.material) continue;

      constexpr auto kInvalidId = ~0;
      const auto materialId = addMaterialInstance(store, subMesh.material);

      renderables.emplace_back(Renderable{
        .mesh = meshInstance->getPrototype().get(),
        .subMeshInstance = subMesh,
        .transformId = transformId,
        .skinOffset = skinOffset.value_or(kInvalidId),
        .materialId = materialId.value_or(kInvalidId),
        .userData = meshInstance->getUserData(),
      });
    }
  }

  return renderables;
}

[[nodiscard]] auto getSceneGrid(const AABB &aabb) {
  Grid grid{aabb};
  return grid.valid() ? grid
                      : Grid{AABB::create(glm::vec3{0.0f}, glm::vec3{0.5f})};
}

FrameGraphResource copyImageToBuffer(FrameGraph &fg,
                                     const FrameGraphResource srcImage,
                                     FrameGraphResource dstBuffer) {
  static constexpr auto kPassName = "ImageToBuffer";
  fg.addCallbackPass(
    kPassName,
    [srcImage, &dstBuffer](FrameGraph::Builder &builder, auto &) {
      PASS_SETUP_ZONE;

      builder.read(srcImage,
                   BindingInfo{.pipelineStage = PipelineStage::Transfer});
      dstBuffer = builder.write(
        dstBuffer, BindingInfo{.pipelineStage = PipelineStage::Transfer});
    },
    [srcImage, dstBuffer](const auto &, FrameGraphPassResources &resources,
                          void *ctx) {
      auto &cb = static_cast<RenderContext *>(ctx)->commandBuffer;
      RHI_GPU_ZONE(cb, kPassName);

      cb.copyImage(*resources.get<FrameGraphTexture>(srcImage).texture,
                   *resources.get<FrameGraphBuffer>(dstBuffer).buffer,
                   rhi::ImageAspect::Color);
    });
  return dstBuffer;
}

} // namespace

//
// WorldRenderer class:
//

WorldRenderer::WorldRenderer(gfx::CubemapConverter &cc)
    : m_renderDevice{cc.getRenderDevice()}, m_cubemapConverter{cc} {
  m_commonSamplers = {
    .point = m_renderDevice.getSampler({
      .magFilter = rhi::TexelFilter::Nearest,
      .minFilter = rhi::TexelFilter::Nearest,
      .mipmapMode = rhi::MipmapMode::Nearest,

      .addressModeS = rhi::SamplerAddressMode::ClampToEdge,
      .addressModeT = rhi::SamplerAddressMode::ClampToEdge,
      .addressModeR = rhi::SamplerAddressMode::ClampToEdge,
    }),
    .bilinear = m_renderDevice.getSampler({
      .magFilter = rhi::TexelFilter::Linear,
      .minFilter = rhi::TexelFilter::Linear,
      .mipmapMode = rhi::MipmapMode::Nearest,

      .addressModeS = rhi::SamplerAddressMode::ClampToEdge,
      .addressModeT = rhi::SamplerAddressMode::ClampToEdge,
      .addressModeR = rhi::SamplerAddressMode::ClampToEdge,
    }),
    .shadow = m_renderDevice.getSampler({
      .magFilter = rhi::TexelFilter::Linear,
      .minFilter = rhi::TexelFilter::Linear,
      .mipmapMode = rhi::MipmapMode::Nearest,

      .addressModeS = rhi::SamplerAddressMode::ClampToBorder,
      .addressModeT = rhi::SamplerAddressMode::ClampToBorder,

      .compareOp = rhi::CompareOp::LessOrEqual,

      .borderColor = rhi::BorderColor::OpaqueBlack,
    }),
    .omniShadow = m_renderDevice.getSampler({
      .magFilter = rhi::TexelFilter::Linear,
      .minFilter = rhi::TexelFilter::Linear,
      .mipmapMode = rhi::MipmapMode::Nearest,

      .addressModeS = rhi::SamplerAddressMode::ClampToEdge,
      .addressModeT = rhi::SamplerAddressMode::ClampToEdge,
      .addressModeR = rhi::SamplerAddressMode::ClampToEdge,

      .compareOp = rhi::CompareOp::LessOrEqual,
    }),
  };

  getRenderDevice().execute(
    [this](auto &cb) { m_brdf = m_ibl.generateBRDF(cb); });
}

rhi::RenderDevice &WorldRenderer::getRenderDevice() const {
  return m_renderDevice;
}

TransientResources::MemoryStats
WorldRenderer::getTransientResourcesStats() const {
  return m_transientResources.getStats();
}

#define TECHNIQUES                                                             \
  &m_cubemapConverter, &m_ibl, &m_tiledLighting, &m_shadowRenderer,            \
    &m_globalIllumination, &m_gBufferPass, &m_decalPass,                       \
    &m_deferredLightingPass, &m_transparencyPass, &m_transmissionPass,         \
    &m_skyboxPass, &m_weightedBlendedPass, &m_wireframePass,                   \
    &m_debugNormalPass, &m_ssao, &m_ssr, &m_bloom, &m_eyeAdaptation,           \
    &m_tonemapPass, &m_fxaa, &m_postProcessor, &m_finalPass, &m_debugDrawPass, \
    &m_infiniteGridPass, &m_outlineRenderer, &m_blur, &m_blit

uint32_t WorldRenderer::countPipelines(PipelineGroups flags) const {
  uint32_t n{0};
  for (auto technique : std::initializer_list<const Technique *>{TECHNIQUES}) {
    n += technique->count(flags);
  }
  return n;
}
void WorldRenderer::clearPipelines(PipelineGroups flags) {
  for (auto technique : std::initializer_list<Technique *>{TECHNIQUES}) {
    technique->clear(flags);
  }
}

#undef TECHNIQUES

SkyLight WorldRenderer::createSkyLight(TextureResourceHandle source) {
  assert(source && bool(source));
  ZoneScopedN("CreateSkyLight");

  SkyLight skyLight{{}};
  m_renderDevice.execute([&](rhi::CommandBuffer &cb) {
    skyLight.source = source;

    switch (source->getType()) {
    case rhi::TextureType::Texture2D:
      skyLight.environment = m_renderDevice.makeShared<rhi::Texture>(
        m_cubemapConverter.equirectangularToCubemap(cb, *source));
      break;
    case rhi::TextureType::TextureCube:
      skyLight.environment = source.handle();
      break;

    default:
      assert(false);
    }

    skyLight.specular = m_renderDevice.makeShared<rhi::Texture>(
      m_ibl.prefilterEnvMap(cb, *skyLight.environment));
    skyLight.diffuse = m_renderDevice.makeShared<rhi::Texture>(
      m_ibl.generateIrradiance(cb, *skyLight.environment));
  });
  return skyLight;
}

void WorldRenderer::drawFrame(rhi::CommandBuffer &commandBuffer,
                              const WorldView &worldView, float deltaTime,
                              DebugOutput *debugOutput) {
  ZoneScopedN("WorldRenderer::DrawFrame");

  FrameGraph fg;
  fg.reserve(100, 100);
  FrameGraphBlackboard blackboard;
  {
    ZoneScopedN("FrameGraph::Setup");
    m_dummyResources.embedDummyResources(fg, blackboard);

    uploadFrameBlock(fg, blackboard,
                     {
                       .time = m_time,
                       .deltaTime = deltaTime,
                     });

    RenderableStore renderableStore{{
      // Reserve size (to reduce reallocations).
      .numTransforms = worldView.meshes.size() + worldView.decals.size(),
      .numJoints = 1024,
    }};

    const auto renderables =
      buildRenderables(renderableStore, worldView.meshes);
    const auto decalRenderables =
      buildRenderables(renderableStore, worldView.decals);

    const auto minOffsetAlignment =
      m_renderDevice.getDeviceLimits().minStorageBufferOffsetAlignment;

    auto &[modelMatrices, joints, propertyGroups] = renderableStore;
    auto propertyBlock = mergeProperties(propertyGroups, minOffsetAlignment);

    uploadTransforms(fg, blackboard, std::move(modelMatrices));
    uploadSkins(fg, blackboard, std::move(joints));
    uploadMaterialProperties(fg, blackboard, std::move(propertyBlock.buffer));

    blackboard.add<BRDF>(importTexture(fg, "BRDF LUT", &m_brdf));

    const auto sceneGrid = getSceneGrid(worldView.aabb);
    for (const auto &sceneView : worldView.sceneViews) {
      if (!sceneView.target) continue;

      if (sceneView.debugDraw &&
          bool(sceneView.renderSettings.debugFlags & DebugFlags::WorldBounds)) {
        sceneView.debugDraw->addAABB(sceneGrid.aabb);
      }
      // The blackboard is passed by value on purpose.
      // Each sceneView gets it's own blackboard with global nodes
      // (Dummy resources, BRDF LUT...).
      _drawScene(fg, blackboard, sceneView, sceneGrid, worldView.lights,
                 renderables, decalRenderables, propertyBlock.offsets,
                 deltaTime);
    }
  }
  {
    ZoneScopedN("FrameGraph::Compile");
    fg.compile();
  }
  if (debugOutput != nullptr) {
    debugOutput->dot = (std::ostringstream{} << fg).str();
  }
  {
    RenderContext rc{commandBuffer, m_commonSamplers};
    TRACY_GPU_ZONE(rc.commandBuffer, "FrameGraph::Execute");
    fg.execute(&rc, &m_transientResources);
  }
  m_transientResources.update();

  m_time += deltaTime;
}

std::optional<StageError>
WorldRenderer::isValid(const Material &material) const {
  return isValid(m_renderDevice, material);
}
std::optional<StageError> WorldRenderer::isValid(const rhi::RenderDevice &rd,
                                                 const Material &material) {
  CodePair code;

  switch (getDomain(material)) {
  case MaterialDomain::Surface: {
    const auto vertexFormat = Vertex1p1n1st::getVertexFormat();

    const auto &surface = getSurface(material);
    if (surface.lightingMode == LightingMode::Default) {
      switch (surface.blendMode) {
      case BlendMode::Opaque:
      case BlendMode::Masked:
        code =
          GBufferPass::buildShaderCode(rd, vertexFormat.get(), material, false);
        break;
      case BlendMode::Transparent:
      case BlendMode::Add:
      case BlendMode::Modulate:
        code = TransparencyPass::buildShaderCode(rd, vertexFormat.get(),
                                                 material, {}, false);
        break;
      }
    } else {
      code = TransmissionPass::buildShaderCode(rd, vertexFormat.get(), material,
                                               {}, false);
    }
  } break;
  case MaterialDomain::PostProcess: {
    code = {
      .vert = ShaderCodeBuilder{}.buildFromFile("FullScreenTriangle.vert"),
      .frag = PostProcessor::buildFragmentCode(rd, material),
    };
  } break;
  }

  StageError stageError;
  for (auto [shaderType, glsl] : {
         std::pair{rhi::ShaderType::Vertex, &code.vert},
         std::pair{rhi::ShaderType::Fragment, &code.frag},
       }) {
    if (auto spv = rd.compile(shaderType, *glsl); !spv) {
      stageError[shaderType] = spv.error();
    }
  }

  return stageError.empty() ? std::nullopt
                            : std::make_optional(std::move(stageError));
}

void WorldRenderer::_drawScene(FrameGraph &fg, FrameGraphBlackboard blackboard,
                               const SceneView &sceneView,
                               const Grid &sceneGrid,
                               std::span<const Light *> lights,
                               const std::vector<Renderable> &renderables,
                               const std::vector<Renderable> &decalRenderables,
                               const PropertyGroupOffsets &propertyGroupOffsets,
                               float deltaTime) {
  auto &target = sceneView.target;
  const auto resolution = target.getExtent();
  assert(target && resolution);

  ZoneScopedN("DrawScene");

  const auto backbuffer = importTexture(fg, sceneView.name, &target);
  if (auto *buffer = sceneView.userData;
      buffer && buffer->getSize() >=
                  resolution.width * resolution.height * sizeof(uint32_t)) {
    blackboard.add<UserData>().userBuffer = fg.import <FrameGraphBuffer>(
      "UserData",
      FrameGraphBuffer::Desc{
        .type = BufferType::StorageBuffer,
        .stride = sizeof(uint32_t),
        .capacity = buffer->getSize() / sizeof(uint32_t),
      },
      {buffer});
  }

  auto &camera = sceneView.camera;
  blackboard.add<CameraData>(uploadCameraBlock(fg, resolution, camera));

  if (sceneView.skyLight) importSkyLight(fg, blackboard, *sceneView.skyLight);

  // ---

  const auto &settings = sceneView.renderSettings;

  const auto &viewFrustum = camera.getFrustum();
  auto visibleLights = getVisibleLights(lights, viewFrustum);

  const auto directionalLight = getFirstDirectionalLight(visibleLights);

  if (bool(settings.features & RenderFeatures::GI) && directionalLight) {
    m_globalIllumination.update(
      fg, blackboard, sceneGrid, camera, *directionalLight, renderables,
      propertyGroupOffsets, settings.globalIllumination.numPropagations);
  }

  // ---

  auto visibleRenderables = getVisibleRenderables(renderables, viewFrustum);

  m_gBufferPass.addGeometryPass(fg, blackboard, resolution,
                                {
                                  camera,
                                  visibleRenderables,
                                },
                                propertyGroupOffsets);

  if (auto visibleDecalRenderables =
        getVisibleRenderables(decalRenderables, viewFrustum);
      !visibleDecalRenderables.empty()) {
    m_decalPass.addGeometryPass(fg, blackboard,
                                {
                                  camera,
                                  visibleDecalRenderables,
                                },
                                propertyGroupOffsets);
  }

  if (bool(settings.features & RenderFeatures::SSAO))
    m_ssao.addPass(fg, blackboard, m_blur, settings.ssao);

  const auto hasLights = !visibleLights.empty();

  auto shadowMapIndices =
    m_shadowRenderer.update(fg, blackboard, camera, visibleLights, renderables,
                            propertyGroupOffsets, settings.shadow);

  uploadLights(fg, blackboard, std::move(visibleLights),
               std::move(shadowMapIndices));

  if (bool(settings.features & RenderFeatures::LightCulling) && hasLights) {
    m_tiledLighting.cullLights(fg, blackboard, kTileSize);
  }

  const LightingSettings lightingSettings{
    .ambientLight = settings.ambientLight,
    .IBLIntensity = settings.IBLIntensity,
    .GIIntensity = settings.globalIllumination.intensity,
  };
  const auto hasSoftShadows =
    bool(settings.features & RenderFeatures::SoftShadows);

  auto &sceneColor = blackboard.add<SceneColorData>();
  sceneColor.HDR = m_deferredLightingPass.addPass(
    fg, blackboard, lightingSettings, hasSoftShadows,
    bool(settings.debugFlags & DebugFlags::IrradianceOnly));

  sceneColor.HDR = m_skyboxPass.addPass(fg, blackboard, sceneColor.HDR);

  if (bool(settings.features & RenderFeatures::SSR)) {
    auto reflections = m_ssr.addPass(fg, blackboard);
    reflections = m_blur.addTwoPassGaussianBlur(fg, reflections, 1.5f);
    sceneColor.HDR = m_blit.addColor(fg, sceneColor.HDR, reflections);
  }

  const auto transmission = m_transmissionPass.addGeometryPass(
    fg, blackboard, sceneColor.HDR,
    {
      camera,
      visibleRenderables,
    },
    propertyGroupOffsets, lightingSettings, hasSoftShadows);
  if (transmission) {
    sceneColor.HDR = m_blit.mix(fg, sceneColor.HDR, *transmission);
  }

  // Transparency before SSR causes weird artifacts (lack of depth data).
  if constexpr (kUseWeightedBlendedTechnique) {
    m_weightedBlendedPass.addGeometryPass(fg, blackboard,
                                          {
                                            camera,
                                            visibleRenderables,
                                          },
                                          propertyGroupOffsets,
                                          lightingSettings, hasSoftShadows);
    m_weightedBlendedPass.compose(fg, blackboard);
  } else {
    const auto transparency = m_transparencyPass.addGeometryPass(
      fg, blackboard, sceneColor.HDR,
      {
        camera,
        visibleRenderables,
      },
      propertyGroupOffsets, lightingSettings, hasSoftShadows);
    if (transparency) {
      sceneColor.HDR = m_blit.mix(fg, sceneColor.HDR, *transparency);
    }
  }

  if (bool(settings.features & RenderFeatures::Bloom)) {
    m_bloom.resample(fg, blackboard, settings.bloom.radius);
  }
  if (bool(settings.features & RenderFeatures::EyeAdaptation)) {
    const auto sceneUniqueId = std::bit_cast<uint64_t>(&target);
    m_eyeAdaptation.compute(fg, blackboard, settings.adaptiveExposure,
                            sceneUniqueId, deltaTime);
  }

  sceneColor.LDR =
    m_tonemapPass.addPass(fg, blackboard, settings.tonemap, settings.exposure,
                          settings.bloom.strength);

  // ---

  if (bool(settings.debugFlags & DebugFlags::InfiniteGrid)) {
    sceneColor.LDR = m_infiniteGridPass.addPass(fg, blackboard, sceneColor.LDR);
  }
  if (bool(settings.debugFlags & DebugFlags::Wireframe)) {
    sceneColor.LDR = m_wireframePass.addGeometryPass(
      fg, blackboard, sceneColor.LDR, {camera, visibleRenderables});
  }
  if (bool(settings.debugFlags & DebugFlags::VertexNormal)) {
    sceneColor.LDR = m_debugNormalPass.addGeometryPass(
      fg, blackboard, sceneColor.LDR, {camera, visibleRenderables});
  }

  if (bool(settings.features & RenderFeatures::GI) &&
      bool(settings.debugFlags & DebugFlags::VPL)) {
    sceneColor.LDR =
      m_globalIllumination.addDebugPass(fg, blackboard, sceneColor.LDR);
  }

  if (auto dd = sceneView.debugDraw; dd && !dd->empty()) {
    sceneColor.LDR =
      m_debugDrawPass.addGeometryPass(fg, blackboard, sceneColor.LDR, *dd);
  }

  if (bool(settings.features & RenderFeatures::FXAA))
    sceneColor.LDR = m_fxaa.addPass(fg, blackboard, sceneColor.LDR);

  if (bool(settings.debugFlags & DebugFlags::CascadeSplits)) {
    sceneColor.LDR =
      m_shadowRenderer.visualizeCascades(fg, blackboard, sceneColor.LDR);
  }
  if (bool(settings.debugFlags & DebugFlags::LightHeatmap)) {
    sceneColor.LDR =
      m_tiledLighting.addDebugOverlay(fg, blackboard, m_blit, sceneColor.LDR);
  }

  // ---

  if (bool(settings.features & RenderFeatures::CustomPostprocess)) {
    for (const auto &effect : sceneView.postProcessEffects) {
      if (effect.isEnabled()) {
        sceneColor.LDR =
          m_postProcessor.addPass(fg, blackboard, sceneColor.LDR, effect);
      }
    }
  }

  sceneColor.LDR = m_outlineRenderer.addOutlines(
    fg, blackboard, {camera, visibleRenderables}, sceneColor.LDR);

  m_finalPass.compose(fg, blackboard, settings.outputMode, backbuffer);

  // ---

  if (auto *d = blackboard.try_get<UserData>(); d)
    copyImageToBuffer(fg, d->target, d->userBuffer);
}

} // namespace gfx
