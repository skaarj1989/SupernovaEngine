#include "renderer/ShadowRenderer.hpp"

#include "FrameGraphCommon.hpp"
#include "renderer/FrameGraphTexture.hpp"
#include "FrameGraphResourceAccess.hpp"

#include "FrameGraphData/DummyResources.hpp"
#include "FrameGraphData/Frame.hpp"
#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/Transforms.hpp"
#include "FrameGraphData/Skins.hpp"
#include "FrameGraphData/MaterialProperties.hpp"
#include "FrameGraphData/GBuffer.hpp"
#include "FrameGraphData/ShadowMap.hpp"

#include "UploadInstances.hpp"
#include "UploadCameraBlock.hpp"
#include "UploadShadowBlock.hpp"

#include "math/CollisionDetection.hpp"

#include "MaterialShader.hpp"
#include "BatchBuilder.hpp"
#include "ShadowCascadesBuilder.hpp"

#include "RenderContext.hpp"

#include <ranges>

namespace gfx {

namespace {

constexpr auto kDepthFormat = rhi::PixelFormat::Depth16;

[[nodiscard]] auto createDebugPipeline(rhi::RenderDevice &rd) {
  ShaderCodeBuilder shaderCodeBuilder;

  // clang-format off
  return rhi::GraphicsPipeline::Builder{}
    .setColorFormats({rhi::PixelFormat::RGBA8_UNorm})
    .setInputAssembly({})
    .addShader(rhi::ShaderType::Vertex,
               shaderCodeBuilder.buildFromFile("FullScreenTriangle.vert"))
    .addShader(rhi::ShaderType::Fragment,
               shaderCodeBuilder.buildFromFile("VisualizeCascadeSplits.frag"))

    .setDepthStencil({
      .depthTest = false,
      .depthWrite = false,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = rhi::CullMode::Front,
    })
    .setBlending(0, {
      .enabled = true,
      .srcColor = rhi::BlendFactor::SrcAlpha,
      .dstColor = rhi::BlendFactor::OneMinusSrcAlpha,
      .colorOp = rhi::BlendOp::Add,

      .srcAlpha = rhi::BlendFactor::One,
      .dstAlpha = rhi::BlendFactor::One,
      .alphaOp = rhi::BlendOp::Add,
    })
    .build(rd);
  // clang-format on
}

struct SortByDistance {
  glm::vec3 origin;

  inline bool operator()(const Light *a, const Light *b) const noexcept {
    const auto distanceA = glm::distance(origin, a->position);
    const auto distanceB = glm::distance(origin, b->position);
    return distanceB > distanceA; // Front to back.
  }
};

[[nodiscard]] auto getFirstDirectionalLight(std::span<const Light *> lights) {
  const auto it = std::ranges::find_if(lights, [](const auto *light) {
    return light->type == LightType::Directional && light->castsShadow;
  });
  return it != lights.cend() ? *it : nullptr;
}
[[nodiscard]] auto
getShadowCastingLights(std::span<const Light *> visibleLights,
                       LightType lightType) {
  ZoneScoped;

  std::vector<const Light *> result;
  result.reserve(visibleLights.size());
  const auto match = [lightType](const Light *light) {
    return light->type == lightType && light->castsShadow;
  };
  std::ranges::copy_if(visibleLights, std::back_inserter(result), match);
  return result;
}

[[nodiscard]] auto batchCompatible(const Batch &b, const Renderable &r) {
  return sameGeometry(b, r) && sameMaterial(b, r) && sameTextures(b, r);
}

[[nodiscard]] bool isShadowCaster(const Renderable &r) {
  return r.subMeshInstance.material.castsShadow();
}

template <typename Func>
  requires std::is_invocable_v<Func, const AABB &>
[[nodiscard]] auto
getVisibleShadowCasters(std::span<const Renderable> renderables,
                        Func isVisible) {
  ZoneScoped;

  std::vector<const Renderable *> result;
  result.reserve(renderables.size());
  for (const auto &renderable : renderables) {
    if (isShadowCaster(renderable) &&
        isVisible(renderable.subMeshInstance.aabb)) {
      result.emplace_back(&renderable);
    }
  }
  return result;
}

void read(FrameGraph::Builder &builder, const FrameGraphBlackboard &blackboard,
          CameraData cameraData, std::optional<FrameGraphResource> instances) {
  read(builder, blackboard.get<FrameData>());
  read(builder, cameraData);

  const auto &dummyResources = blackboard.get<DummyResourcesData>();
  readInstances(builder, instances, dummyResources);
  read(builder, blackboard.try_get<TransformData>(), dummyResources);
  read(builder, blackboard.try_get<SkinData>(), dummyResources);

  if (auto *d = blackboard.try_get<MaterialPropertiesData>(); d) {
    read(builder, *d);
  }
}

template <typename Func>
void render(RenderContext &rc, const Batches &batches, Func f) {
  BaseGeometryPassInfo passInfo{
    .depthFormat = rhi::getDepthFormat(*rc.framebufferInfo),
  };

  rc.commandBuffer.beginRendering(*rc.framebufferInfo);
  for (const auto &batch : batches) {
    if (const auto *pipeline = f(adjust(passInfo, batch)); pipeline) {
      render(rc, *pipeline, batch);
    }
  }
  endRendering(rc);
}

#define RENDER(rc, batches)                                                    \
  render(rc, batches, [this](const auto &arg) -> decltype(auto) {              \
    return _getPipeline(arg);                                                  \
  });

} // namespace

//
// ShadowRenderer class:
//

ShadowRenderer::ShadowRenderer(rhi::RenderDevice &rd)
    : rhi::RenderPass<ShadowRenderer>{rd},
      m_debugPipeline{createDebugPipeline(rd)} {}

uint32_t ShadowRenderer::count(PipelineGroups flags) const {
  uint32_t n{0};
  if (bool(flags & PipelineGroups::BuiltIn)) {
    n += 1; // Single debug pipeline (visualize cascade splits).
  }
  if (bool(flags & PipelineGroups::SurfaceMaterial)) {
    n += BasePass::count();
  }
  return n;
}
void ShadowRenderer::clear(PipelineGroups flags) {
  if (bool(flags & PipelineGroups::SurfaceMaterial)) BasePass::clear();
}

ShadowMapIndices ShadowRenderer::update(
  FrameGraph &fg, FrameGraphBlackboard &blackboard,
  const PerspectiveCamera &camera, std::span<const Light *> visibleLights,
  std::span<const Renderable> renderables,
  const PropertyGroupOffsets &propertyGroupOffsets, const Settings &settings) {
  ZoneScoped;

  auto &shadowMapData = blackboard.add<ShadowMapData>();

  ShadowMapIndices shadowMapIndices;
  constexpr auto kNumLightTypes = 3;
  shadowMapIndices.reserve(kNumLightTypes);

  ShadowBlock shadowBlock;

  // -- Directional Light:

  if (const auto *directionalLight = getFirstDirectionalLight(visibleLights);
      directionalLight) {
    shadowBlock.cascades = _buildCascadedShadowMaps(
      fg, blackboard, camera, *directionalLight, renderables,
      propertyGroupOffsets, settings.cascadedShadowMaps);

    shadowMapIndices[LightType::Directional].emplace_back(directionalLight, 0u);
  }

  // -- Spot Lights:

  if (auto spotLights = getShadowCastingLights(visibleLights, LightType::Spot);
      !spotLights.empty()) {
    std::ranges::sort(spotLights, SortByDistance{camera.getPosition()});

    shadowBlock.spotLightViewProjections = _buildSpotLightShadowMaps(
      fg, blackboard, spotLights, shadowMapIndices[LightType::Spot],
      renderables, propertyGroupOffsets, settings.spotLightShadowMaps);
  }
  if (!shadowBlock.cascades.empty() ||
      !shadowBlock.spotLightViewProjections.empty()) {
    shadowMapData.shadowBlock = uploadShadowBlock(fg, shadowBlock);
  }

  // -- Point Lights:

  if (auto pointLights =
        getShadowCastingLights(visibleLights, LightType::Point);
      !pointLights.empty()) {
    std::ranges::sort(pointLights, SortByDistance{camera.getPosition()});

    _buildOmniLightShadowMaps(fg, blackboard, pointLights,
                              shadowMapIndices[LightType::Point], renderables,
                              propertyGroupOffsets, settings.omniShadowMaps);
  }
  return shadowMapIndices;
}

FrameGraphResource
ShadowRenderer::visualizeCascades(FrameGraph &fg,
                                  const FrameGraphBlackboard &blackboard,
                                  FrameGraphResource target) const {
  ZoneScoped;

  constexpr auto kPassName = "VisualizeCascades";

  const auto shadowBlock = blackboard.get<ShadowMapData>().shadowBlock;
  if (!shadowBlock) return target;

  fg.addCallbackPass(
    kPassName,
    [&blackboard, &target, &shadowBlock](FrameGraph::Builder &builder, auto &) {
      read(builder, blackboard.get<CameraData>(),
           PipelineStage::FragmentShader);
      readShadowBlock(builder, *shadowBlock);
      builder.read(blackboard.get<GBufferData>().depth,
                   TextureRead{
                     .binding =
                       {
                         .location = {.set = 2, .binding = 1},
                         .pipelineStage = PipelineStage::FragmentShader,
                       },
                     .type = TextureRead::Type::SampledImage,
                   });

      target = builder.write(target, Attachment{.index = 0});
    },
    [this](const auto &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, kPassName)
      renderFullScreenPostProcess(rc, m_debugPipeline);
    });

  return target;
}

//
// (private):
//

std::vector<Cascade> ShadowRenderer::_buildCascadedShadowMaps(
  FrameGraph &fg, FrameGraphBlackboard &blackboard,
  const PerspectiveCamera &camera, const Light &light,
  std::span<const Renderable> renderables,
  const PropertyGroupOffsets &propertyGroupOffsets,
  const Settings::CascadedShadowMaps &settings) {
  assert(light.type == LightType::Directional);
  ZoneScoped;

  const auto cascades =
    buildCascades(camera, light.direction, settings.numCascades,
                  settings.lambda, settings.shadowMapSize);
  auto &shadowMaps = blackboard.get<ShadowMapData>().cascadedShadowMaps;

  for (auto [i, cascade] : std::views::enumerate(cascades)) {
    auto visibleShadowCasters = getVisibleShadowCasters(
      renderables, [frustum = Frustum{cascade.lightView.viewProjection}](
                     const AABB &aabb) { return frustum.testAABB(aabb); });
    sortByMaterial(visibleShadowCasters);

    // The first iteration is responsible for creation of the shadowmap.
    // (Texture2DArray)
    shadowMaps = _addCascadePass(
      fg, blackboard, i, shadowMaps, cascade.lightView,
      std::move(visibleShadowCasters), propertyGroupOffsets, settings);
  }
  assert(shadowMaps);

  return cascades;
}

FrameGraphResource ShadowRenderer::_addCascadePass(
  FrameGraph &fg, const FrameGraphBlackboard &blackboard, uint32_t cascadeIndex,
  std::optional<FrameGraphResource> cascadedShadowMaps,
  const RawCamera &lightView, std::vector<const Renderable *> &&renderables,
  const PropertyGroupOffsets &propertyGroupOffsets,
  const Settings::CascadedShadowMaps &settings) {
  assert(cascadeIndex < settings.numCascades);
  ZoneScoped;

  const auto cameraBlock = uploadCameraBlock(
    fg, {settings.shadowMapSize, settings.shadowMapSize}, lightView);

  std::vector<GPUInstance> gpuInstances;
  auto batches = buildBatches(gpuInstances, renderables, propertyGroupOffsets,
                              batchCompatible);

  std::optional<FrameGraphResource> instances;
  if (!batches.empty())
    instances = uploadInstances(fg, std::move(gpuInstances));

  const auto passName = std::format("CSM #{}", cascadeIndex);

  struct Data {
    FrameGraphResource shadowMaps;
  };
  const auto [shadowMaps] = fg.addCallbackPass<Data>(
    passName,
    [&](FrameGraph::Builder &builder, Data &data) {
      read(builder, blackboard, CameraData{cameraBlock}, instances);

      if (cascadeIndex == 0) {
        assert(!cascadedShadowMaps.has_value());
        cascadedShadowMaps = builder.create<FrameGraphTexture>(
          "CascadedShadowMaps",
          {
            .extent = {settings.shadowMapSize, settings.shadowMapSize},
            .format = kDepthFormat,
            .layers = settings.numCascades,
            .usageFlags =
              rhi::ImageUsage::RenderTarget | rhi::ImageUsage::Sampled,
          });
      }
      data.shadowMaps =
        builder.write(*cascadedShadowMaps, Attachment{
                                             .layer = cascadeIndex,
                                             .clearValue = ClearValue::One,
                                           });
    },
    [this, passName, batches = std::move(batches)](
      const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, passName.c_str())
      RENDER(rc, batches)
    });

  return shadowMaps;
}

std::vector<glm::mat4> ShadowRenderer::_buildSpotLightShadowMaps(
  FrameGraph &fg, FrameGraphBlackboard &blackboard,
  std::span<const Light *> lights, std::vector<LightShadowPair> &shadowIndices,
  std::span<const Renderable> renderables,
  const PropertyGroupOffsets &propertyGroupOffsets,
  const Settings::SpotLightShadowMaps &settings) {
  assert(!lights.empty());
  ZoneScoped;

  auto &shadowMaps = blackboard.get<ShadowMapData>().spotLightShadowMaps;
  std::vector<glm::mat4> shadowMatrices;

  for (auto [i, spotLight] : std::views::enumerate(lights)) {
    assert(spotLight && spotLight->type == LightType::Spot);
    if (i >= settings.maxNumShadows) break;

    auto lightView = buildSpotLightMatrix(*spotLight);
    auto visibleShadowCasters = getVisibleShadowCasters(
      renderables, [frustum = Frustum{lightView.viewProjection}](
                     const AABB &aabb) { return frustum.testAABB(aabb); });
    if (visibleShadowCasters.empty()) continue;

    sortByMaterial(visibleShadowCasters);

    shadowMaps = _addSpotLightPass(fg, blackboard, i, shadowMaps, lightView,
                                   std::move(visibleShadowCasters),
                                   propertyGroupOffsets, settings);
    shadowMatrices.emplace_back(std::move(lightView.viewProjection));
    shadowIndices.emplace_back(spotLight, i);

    ++i;
  }
  return shadowMatrices;
}

FrameGraphResource ShadowRenderer::_addSpotLightPass(
  FrameGraph &fg, const FrameGraphBlackboard &blackboard, uint32_t index,
  std::optional<FrameGraphResource> shadowMaps, const RawCamera &lightView,
  std::vector<const Renderable *> &&renderables,
  const PropertyGroupOffsets &propertyGroupOffsets,
  const Settings::SpotLightShadowMaps &settings) {
  ZoneScoped;

  const auto cameraBlock = uploadCameraBlock(
    fg, {settings.shadowMapSize, settings.shadowMapSize}, lightView);

  std::vector<GPUInstance> gpuInstances;
  auto batches = buildBatches(gpuInstances, renderables, propertyGroupOffsets,
                              batchCompatible);

  std::optional<FrameGraphResource> instances;
  if (!batches.empty())
    instances = uploadInstances(fg, std::move(gpuInstances));

  const auto passName = std::format("SpotLightShadowPass #{}", index);

  struct Data {
    FrameGraphResource shadowMaps;
  };
  const auto [output] = fg.addCallbackPass<Data>(
    passName,
    [&](FrameGraph::Builder &builder, Data &data) {
      read(builder, blackboard, CameraData{cameraBlock}, instances);

      if (index == 0) {
        assert(!shadowMaps);
        shadowMaps = builder.create<FrameGraphTexture>(
          "SpotLightShadowMaps",
          {
            .extent = {settings.shadowMapSize, settings.shadowMapSize},
            .format = kDepthFormat,
            .layers = settings.maxNumShadows,
            .usageFlags =
              rhi::ImageUsage::RenderTarget | rhi::ImageUsage::Sampled,
          });
      }
      data.shadowMaps =
        builder.write(*shadowMaps, Attachment{
                                     .layer = index,
                                     .clearValue = ClearValue::One,
                                   });
    },
    [this, passName, batches = std::move(batches)](
      const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, passName.c_str())
      RENDER(rc, batches)
    });

  return output;
}

void ShadowRenderer::_buildOmniLightShadowMaps(
  FrameGraph &fg, FrameGraphBlackboard &blackboard,
  std::span<const Light *> visibleLights, std::vector<LightShadowPair> &indices,
  std::span<const Renderable> allRenderables,
  const PropertyGroupOffsets &propertyGroupOffsets,
  const Settings::OmniShadowMaps &settings) {
  ZoneScoped;

  auto &shadowMaps = blackboard.get<ShadowMapData>().omniShadowMaps;
  for (auto [i, pointLight] : std::views::enumerate(visibleLights)) {
    assert(pointLight && pointLight->type == LightType::Point);
    if (i >= settings.maxNumShadows) break;

    auto shadowCastersInRange = getVisibleShadowCasters(
      allRenderables, [sphere = toSphere(*pointLight)](const AABB &aabb) {
        return intersects(sphere, aabb);
      });
    if (shadowCastersInRange.empty()) continue;

    for (auto face = 0; face < 6; ++face) {
      shadowMaps = _addOmniLightPass(
        fg, blackboard, i, static_cast<rhi::CubeFace>(face), shadowMaps,
        *pointLight, shadowCastersInRange, propertyGroupOffsets, settings);
    }
    indices.emplace_back(pointLight, i);

    ++i;
  }
}

FrameGraphResource ShadowRenderer::_addOmniLightPass(
  FrameGraph &fg, FrameGraphBlackboard &blackboard, uint32_t index,
  rhi::CubeFace face, std::optional<FrameGraphResource> shadowMaps,
  const Light &light, std::span<const Renderable *> renderablesInRange,
  const PropertyGroupOffsets &propertyGroupOffsets,
  const Settings::OmniShadowMaps &settings) {
  assert(light.type == LightType::Point);
  ZoneScoped;

  const auto lightView =
    buildPointLightMatrix(face, light.position, light.range);
  const Frustum frustum{lightView.viewProjection};

  const auto cameraBlock = uploadCameraBlock(
    fg, {settings.shadowMapSize, settings.shadowMapSize}, lightView);

  std::vector<const Renderable *> visibleShadowCasters;
  visibleShadowCasters.reserve(renderablesInRange.size());
  std::ranges::copy_if(renderablesInRange,
                       std::back_inserter(visibleShadowCasters),
                       [&frustum](const Renderable *r) {
                         return frustum.testAABB(r->subMeshInstance.aabb);
                       });
  sortByMaterial(visibleShadowCasters);

  std::vector<GPUInstance> gpuInstances;
  auto batches = buildBatches(gpuInstances, visibleShadowCasters,
                              propertyGroupOffsets, batchCompatible);

  std::optional<FrameGraphResource> instances;
  if (!batches.empty())
    instances = uploadInstances(fg, std::move(gpuInstances));

  const auto passName =
    std::format("OmniShadowPass[#{}, {}]", index, toString(face));

  struct Data {
    FrameGraphResource shadowMaps;
  };
  const auto [output] = fg.addCallbackPass<Data>(
    passName,
    [&](FrameGraph::Builder &builder, Data &data) {
      read(builder, blackboard, CameraData{cameraBlock}, instances);

      if (index == 0 && face == rhi::CubeFace::PositiveX) {
        assert(!shadowMaps.has_value());

        shadowMaps = builder.create<FrameGraphTexture>(
          "OmniShadowMaps",
          {
            .extent = {settings.shadowMapSize, settings.shadowMapSize},
            .format = kDepthFormat,
            .layers = settings.maxNumShadows,
            .cubemap = true,
            .usageFlags =
              rhi::ImageUsage::RenderTarget | rhi::ImageUsage::Sampled,
          });
      }
      data.shadowMaps =
        builder.write(*shadowMaps, Attachment{
                                     .layer = index,
                                     .face = face,
                                     .clearValue = ClearValue::One,
                                   });
    },
    [this, passName, batches = std::move(batches)](
      const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, passName.c_str())
      RENDER(rc, batches)
    });

  return output;
}

rhi::GraphicsPipeline
ShadowRenderer::_createPipeline(const BaseGeometryPassInfo &passInfo) const {
  assert(passInfo.vertexFormat && passInfo.material);
  assert(passInfo.colorFormats.size() == 0);

  auto &rd = getRenderDevice();

  const auto [vertCode, fragCode] =
    buildShaderCode(rd, passInfo.vertexFormat, *passInfo.material);

  return rhi::GraphicsPipeline::Builder{}
    .setDepthFormat(passInfo.depthFormat)
    .setColorFormats({})
    .setInputAssembly(passInfo.vertexFormat->getAttributes())
    .setTopology(passInfo.topology)
    .addShader(rhi::ShaderType::Vertex, vertCode)
    .addShader(rhi::ShaderType::Fragment, fragCode)

    .setDepthStencil({
      .depthTest = true,
      .depthWrite = true,
      .depthCompareOp = rhi::CompareOp::LessOrEqual,
    })
    .setRasterizer({
      .polygonMode = rhi::PolygonMode::Fill,
      .cullMode = rhi::CullMode::Front,
      .depthClampEnable = true,
    })
    .build(rd);
}

CodePair ShadowRenderer::buildShaderCode(const rhi::RenderDevice &rd,
                                         const VertexFormat *vertexFormat,
                                         const Material &material) {
  const auto offsetAlignment =
    rd.getDeviceLimits().minStorageBufferOffsetAlignment;

  CodePair code;

  auto commonDefines = buildDefines(*vertexFormat);
  commonDefines.emplace_back(std::format("DEPTH_PASS {}", 1));

  ShaderCodeBuilder shaderCodeBuilder;

  // -- VertexShader:

  shaderCodeBuilder.setDefines(commonDefines);
  addMaterial(shaderCodeBuilder, material, rhi::ShaderType::Vertex,
              offsetAlignment);
  code.vert = shaderCodeBuilder.buildFromFile("Mesh.vert");

  // -- FragmentShader:

  shaderCodeBuilder.setDefines(commonDefines);
  if (getSurface(material).blendMode == BlendMode::Masked) {
    addMaterial(shaderCodeBuilder, material, rhi::ShaderType::Fragment,
                offsetAlignment);
  } else {
    noMaterial(shaderCodeBuilder);
  }

  code.frag = shaderCodeBuilder.buildFromFile("DepthPass.frag");

  return code;
}

} // namespace gfx
