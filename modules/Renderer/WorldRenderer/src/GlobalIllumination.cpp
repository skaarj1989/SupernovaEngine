#include "renderer/GlobalIllumination.hpp"
#include "math/Math.hpp"
#include "rhi/RenderDevice.hpp"

#include "renderer/VertexFormat.hpp"
#include "renderer/Light.hpp"
#include "renderer/MeshInstance.hpp"
#include "renderer/Grid.hpp"

#include "PerspectiveCamera.hpp"
#include "Transform.hpp"

#include "FrameGraphResourceAccess.hpp"
#include "FrameGraphCommon.hpp"
#include "renderer/FrameGraphTexture.hpp"
#include "UploadInstances.hpp"
#include "UploadCameraBlock.hpp"
#include "UploadSceneGrid.hpp"

#include "FrameGraphData/DummyResources.hpp"
#include "FrameGraphData/Frame.hpp"
#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/Transforms.hpp"
#include "FrameGraphData/Skins.hpp"
#include "FrameGraphData/MaterialProperties.hpp"
#include "FrameGraphData/GBuffer.hpp"
#include "FrameGraphData/GlobalIllumination.hpp"

#include "MaterialShader.hpp"
#include "ShadowCascadesBuilder.hpp"
#include "BatchBuilder.hpp"

#include "RenderContext.hpp"
#include "ShaderCodeBuilder.hpp"

namespace gfx {

namespace {

constexpr auto kStaticRSM = true;

constexpr auto kRSMResolution = 512;
constexpr auto kRSMExtent = rhi::Extent2D{kRSMResolution, kRSMResolution};
constexpr auto kNumVPL = kRSMResolution * kRSMResolution;

constexpr auto kLPVResolution = 64;
constexpr auto kSHFormat = rhi::PixelFormat::RGBA16F;
constexpr auto kLPVFormat = {kSHFormat, kSHFormat, kSHFormat};

constexpr auto kMaxNumPropagations = 12u;

[[nodiscard]] auto createRadianceInjectionPipeline(rhi::RenderDevice &rd) {
  ShaderCodeBuilder shaderCodeBuilder;

  constexpr auto kAdditiveBlending = rhi::BlendState{
    .enabled = true,
    .srcColor = rhi::BlendFactor::One,
    .dstColor = rhi::BlendFactor::One,
    .colorOp = rhi::BlendOp::Add,

    .srcAlpha = rhi::BlendFactor::One,
    .dstAlpha = rhi::BlendFactor::One,
    .alphaOp = rhi::BlendOp::Add,
  };

  return rhi::GraphicsPipeline::Builder{}
    .setInputAssembly({})
    .setTopology(rhi::PrimitiveTopology::PointList)
    .setColorFormats(kLPVFormat)
    .addShader(rhi::ShaderType::Vertex,
               shaderCodeBuilder.buildFromFile("RadianceInjection.vert"))
    .addShader(rhi::ShaderType::Geometry,
               shaderCodeBuilder.buildFromFile("RadianceInjection.geom"))
    .addShader(rhi::ShaderType::Fragment,
               shaderCodeBuilder.buildFromFile("RadianceInjection.frag"))
    .setDepthStencil({.depthTest = false, .depthWrite = false})
    .setRasterizer({.cullMode = rhi::CullMode::None})
    .setBlending(0, kAdditiveBlending)
    .setBlending(1, kAdditiveBlending)
    .setBlending(2, kAdditiveBlending)
    .build(rd);
}
[[nodiscard]] auto createRadiancePropagationPipeline(rhi::RenderDevice &rd) {
  ShaderCodeBuilder shaderCodeBuilder;

  return rhi::GraphicsPipeline::Builder{}
    .setInputAssembly({})
    .setTopology(rhi::PrimitiveTopology::PointList)
    .setColorFormats(kLPVFormat)
    .addShader(rhi::ShaderType::Vertex,
               shaderCodeBuilder.buildFromFile("RadiancePropagation.vert"))
    .addShader(rhi::ShaderType::Geometry,
               shaderCodeBuilder.buildFromFile("RadiancePropagation.geom"))
    .addShader(rhi::ShaderType::Fragment,
               shaderCodeBuilder.buildFromFile("RadiancePropagation.frag"))
    .setDepthStencil({.depthTest = false, .depthWrite = false})
    .setRasterizer({.cullMode = rhi::CullMode::None})
    .setBlending(0, {.enabled = false})
    .setBlending(1, {.enabled = false})
    .setBlending(2, {.enabled = false})
    .build(rd);
}
[[nodiscard]] auto createDebugPipeline(rhi::RenderDevice &rd) {
  ShaderCodeBuilder shaderCodeBuilder;

  return rhi::GraphicsPipeline::Builder{}
    .setInputAssembly({})
    .setTopology(rhi::PrimitiveTopology::PointList)
    .setDepthFormat(rhi::PixelFormat::Depth32F)
    .setColorFormats({rhi::PixelFormat::RGBA8_UNorm})
    .addShader(rhi::ShaderType::Vertex,
               shaderCodeBuilder.buildFromFile("DebugVPL.vert"))
    .addShader(rhi::ShaderType::Geometry,
               shaderCodeBuilder.buildFromFile("DebugVPL.geom"))
    .addShader(rhi::ShaderType::Fragment,
               shaderCodeBuilder.buildFromFile("DebugVPL.frag"))
    .setDepthStencil({.depthTest = true, .depthWrite = false})
    .setRasterizer({.cullMode = rhi::CullMode::None})
    .setBlending(0, {.enabled = false})
    .build(rd);
}

[[nodiscard]] auto getLightView(const Grid &grid,
                                const PerspectiveCamera &camera,
                                const Light &light) {
  if constexpr (kStaticRSM) {
    const auto target = light.position + light.direction;
    const auto view =
      glm::lookAt(light.position, target, calculateUpVector(light.direction));

    const auto halfExtent = grid.aabb.getExtent() * 0.5f;
    auto projection = glm::ortho(-halfExtent.x, halfExtent.x, -halfExtent.y,
                                 halfExtent.y, -halfExtent.z, halfExtent.z);
    projection[1][1] *= -1.0f;

    return RawCamera{
      .view = view,
      .projection = projection,
    };
  } else {
    constexpr auto kNumCascades = 4;
    constexpr auto kCascadeIndex = 0;
    return buildCascades(camera, light.direction, kNumCascades, 0.01f,
                         kRSMResolution)[kCascadeIndex]
      .lightView;
  }
}
[[nodiscard]] auto
getVisibleRenderables(const Frustum &frustum,
                      std::span<const Renderable> renderables) {
  std::vector<const Renderable *> result;
  result.reserve(renderables.size());
  for (const auto &r : renderables) {
    if (frustum.testAABB(r.subMeshInstance.aabb)) {
      result.push_back(std::addressof(r));
    }
  }
  return result;
}
[[nodiscard]] auto batchCompatible(const Batch &b, const Renderable &r) {
  return sameGeometry(b, r) && sameMaterial(b, r) && sameTextures(b, r);
}

void read(FrameGraph::Builder &builder, const ReflectiveShadowMapData &RSM,
          const PipelineStage pipelineStage) {
  builder.read(RSM.position, TextureRead{
                               .binding =
                                 {
                                   .location = {.set = 2, .binding = 0},
                                   .pipelineStage = pipelineStage,
                                 },
                               .type = TextureRead::Type::SampledImage,
                               .imageAspect = rhi::ImageAspect::Color,
                             });
  builder.read(RSM.normal, TextureRead{
                             .binding =
                               {
                                 .location = {.set = 2, .binding = 1},
                                 .pipelineStage = pipelineStage,
                               },
                             .type = TextureRead::Type::SampledImage,
                             .imageAspect = rhi::ImageAspect::Color,
                           });
  builder.read(RSM.flux, TextureRead{
                           .binding =
                             {
                               .location = {.set = 2, .binding = 2},
                               .pipelineStage = pipelineStage,
                             },
                           .type = TextureRead::Type::SampledImage,
                           .imageAspect = rhi::ImageAspect::Color,
                         });
}
void read(FrameGraph::Builder &builder, const LightPropagationVolumesData &LPV,
          const PipelineStage pipelineStage) {
  builder.read(LPV.r, TextureRead{
                        .binding =
                          {
                            .location = {.set = 2, .binding = 3},
                            .pipelineStage = pipelineStage,
                          },
                        .type = TextureRead::Type::SampledImage,
                        .imageAspect = rhi::ImageAspect::Color,
                      });
  builder.read(LPV.g, TextureRead{
                        .binding =
                          {
                            .location = {.set = 2, .binding = 4},
                            .pipelineStage = pipelineStage,
                          },
                        .type = TextureRead::Type::SampledImage,
                        .imageAspect = rhi::ImageAspect::Color,
                      });
  builder.read(LPV.b, TextureRead{
                        .binding =
                          {
                            .location = {.set = 2, .binding = 5},
                            .pipelineStage = pipelineStage,
                          },
                        .type = TextureRead::Type::SampledImage,
                        .imageAspect = rhi::ImageAspect::Color,
                      });
}

} // namespace

//
// Grid struct:
//

Grid::Grid(const AABB &aabb) : aabb{aabb} {
  const auto extent = aabb.getExtent();
  cellSize = max3(extent) / kLPVResolution;
  size = glm::uvec3{extent / cellSize + 0.5f};
}
bool Grid::valid() const {
  return cellSize > 0.0f && glm::all(glm::greaterThan(size, glm::uvec3{0}));
}

//
// GlobalIllumination class:
//

GlobalIllumination::GlobalIllumination(rhi::RenderDevice &rd)
    : rhi::RenderPass<GlobalIllumination>{rd} {
  m_radianceInjectionPipeline = createRadianceInjectionPipeline(rd);
  m_radiancePropagationPipeline = createRadiancePropagationPipeline(rd);
  m_debugPipeline = createDebugPipeline(rd);
}

uint32_t GlobalIllumination::count(const PipelineGroups flags) const {
  uint32_t n{0};
  if (bool(flags & PipelineGroups::BuiltIn)) {
    // radianceInjection + radiancePropagation + debug.
    constexpr auto kNumBuiltInPipelines = 3;
    n += kNumBuiltInPipelines;
  }
  if (bool(flags & PipelineGroups::SurfaceMaterial)) {
    n += BasePass::count();
  }
  return n;
}
void GlobalIllumination::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::SurfaceMaterial)) BasePass::clear();
}

void GlobalIllumination::update(
  FrameGraph &fg, FrameGraphBlackboard &blackboard, const Grid &grid,
  const PerspectiveCamera &camera, const Light &light,
  std::span<const Renderable> renderables,
  const PropertyGroupOffsets &propertyGroupOffsets, uint32_t numPropagations) {
  assert(grid.valid());

  ZoneScopedN("GlobalIllumination");

  const auto lightView = getLightView(grid, camera, light);
  auto visibleRenderables =
    getVisibleRenderables(Frustum{lightView.viewProjection()}, renderables);
  sortByMaterial(visibleRenderables);

  const auto RSM = _addReflectiveShadowMapPass(
    fg, blackboard, lightView, light.color * light.intensity,
    std::move(visibleRenderables), propertyGroupOffsets);
  blackboard.add<ReflectiveShadowMapData>(RSM);

  const auto sceneGridBlock = uploadSceneGrid(fg, grid);
  const auto radiance =
    _addRadianceInjectionPass(fg, sceneGridBlock, RSM, grid.size);

  std::optional<LightPropagationVolumesData> LPV;
  numPropagations = glm::clamp(numPropagations, 1u, kMaxNumPropagations);
  for (auto i = 0u; i < numPropagations; ++i) {
    LPV = _addRadiancePropagationPass(fg, sceneGridBlock,
                                      LPV.value_or(radiance), grid.size, i);
  }
  assert(LPV);
  blackboard.add<GlobalIlluminationData>(sceneGridBlock, *LPV);
}

FrameGraphResource GlobalIllumination::addDebugPass(
  FrameGraph &fg, FrameGraphBlackboard &blackboard, FrameGraphResource target) {
  static constexpr auto kPassName = "DebugVPL";
  ZoneScopedN(kPassName);

  fg.addCallbackPass(
    kPassName,
    [&blackboard, &target](FrameGraph::Builder &builder, auto &) {
      PASS_SETUP_ZONE;

      read(builder, blackboard.get<CameraData>(), PipelineStage::VertexShader);
      builder.read(blackboard.get<GBufferData>().depth,
                   Attachment{.imageAspect = rhi::ImageAspect::Depth});

      const auto &GI = blackboard.get<GlobalIlluminationData>();
      readSceneGrid(builder, GI.sceneGridBlock, PipelineStage::VertexShader);

      const auto &RSM = blackboard.get<ReflectiveShadowMapData>();
      builder.read(RSM.position,
                   TextureRead{
                     .binding =
                       {
                         .location = {.set = 2, .binding = 0},
                         .pipelineStage = PipelineStage::VertexShader,
                       },
                     .type = TextureRead::Type::SampledImage,
                     .imageAspect = rhi::ImageAspect::Color,
                   });
      builder.read(RSM.normal,
                   TextureRead{
                     .binding =
                       {
                         .location = {.set = 2, .binding = 1},
                         .pipelineStage = PipelineStage::VertexShader,
                       },
                     .type = TextureRead::Type::SampledImage,
                     .imageAspect = rhi::ImageAspect::Color,
                   });
      read(builder, GI.LPV, PipelineStage::FragmentShader);

      target = builder.write(
        target, Attachment{.index = 0, .imageAspect = rhi::ImageAspect::Color});
    },
    [this](const auto &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, commonSamplers, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      sets[0][3] = rhi::bindings::SeparateSampler{commonSamplers.bilinear};

      cb.bindPipeline(m_debugPipeline);
      bindDescriptorSets(rc, m_debugPipeline);
      cb.beginRendering(*framebufferInfo)
        .pushConstants(rhi::ShaderStages::Vertex, 0, &kRSMResolution)
        .draw({
          .topology = rhi::PrimitiveTopology::PointList,
          .numVertices = kNumVPL,
        });
      endRendering(rc);
    });

  return target;
}

CodePair GlobalIllumination::buildShaderCode(const rhi::RenderDevice &rd,
                                             const VertexFormat *vertexFormat,
                                             const Material &material) {
  const auto offsetAlignment =
    rd.getDeviceLimits().minStorageBufferOffsetAlignment;

  CodePair code;

  const auto commonDefines = buildDefines(*vertexFormat);

  ShaderCodeBuilder shaderCodeBuilder;

  // -- VertexShader:

  shaderCodeBuilder.setDefines(commonDefines);
  addMaterial(shaderCodeBuilder, material, rhi::ShaderType::Vertex,
              offsetAlignment);
  code.vert = shaderCodeBuilder.buildFromFile("Mesh.vert");

  // -- FragmentShader:

  shaderCodeBuilder.setDefines(commonDefines);
  addMaterial(shaderCodeBuilder, material, rhi::ShaderType::Fragment,
              offsetAlignment);
  code.frag = shaderCodeBuilder.buildFromFile("ReflectiveShadowMapPass.frag");

  return code;
}

//
// (private):
//

ReflectiveShadowMapData GlobalIllumination::_addReflectiveShadowMapPass(
  FrameGraph &fg, FrameGraphBlackboard &blackboard, const RawCamera &lightView,
  const glm::vec3 lightIntensity, std::vector<const Renderable *> &&renderables,
  const PropertyGroupOffsets &propertyGroupOffsets) {
  static constexpr auto kPassName = "ReflectiveShadowMap";
  ZoneScopedN(kPassName);

  const auto cameraBlock = uploadCameraBlock(fg, kRSMExtent, lightView);

  std::vector<GPUInstance> gpuInstances;
  auto batches = buildBatches(gpuInstances, renderables, propertyGroupOffsets,
                              batchCompatible);
  const auto instances = uploadInstances(fg, std::move(gpuInstances));

  const auto RSMData = fg.addCallbackPass<ReflectiveShadowMapData>(
    kPassName,
    [&blackboard, cameraBlock, instances](FrameGraph::Builder &builder,
                                          ReflectiveShadowMapData &data) {
      PASS_SETUP_ZONE;

      read(builder, blackboard.get<FrameData>());
      read(builder, CameraData{cameraBlock});

      const auto &dummyResources = blackboard.get<DummyResourcesData>();
      readInstances(builder, instances, dummyResources);
      read(builder, blackboard.try_get<TransformData>(), dummyResources);
      read(builder, blackboard.try_get<SkinData>(), dummyResources);

      if (auto d = blackboard.try_get<MaterialPropertiesData>(); d) {
        read(builder, *d);
      }

      data.depth = builder.create<FrameGraphTexture>(
        "RSM/Depth", {
                       .extent = kRSMExtent,
                       .format = rhi::PixelFormat::Depth16,
                       .usageFlags = rhi::ImageUsage::RenderTarget,
                     });
      data.depth =
        builder.write(data.depth, Attachment{
                                    .imageAspect = rhi::ImageAspect::Depth,
                                    .clearValue = ClearValue::One,
                                  });

      const auto kGBufferDesc = FrameGraphTexture::Desc{
        .extent = kRSMExtent,
        .format = rhi::PixelFormat::RGBA16F,
        .usageFlags = rhi::ImageUsage::RenderTarget | rhi::ImageUsage::Sampled,
      };
      data.position =
        builder.create<FrameGraphTexture>("RSM/Position", kGBufferDesc);
      data.position = builder.write(
        data.position, Attachment{
                         .index = 0,
                         .imageAspect = rhi::ImageAspect::Color,
                         .clearValue = ClearValue::TransparentBlack,
                       });

      data.normal =
        builder.create<FrameGraphTexture>("RSM/Normal", kGBufferDesc);
      data.normal =
        builder.write(data.normal, Attachment{
                                     .index = 1,
                                     .imageAspect = rhi::ImageAspect::Color,
                                     .clearValue = ClearValue::TransparentBlack,
                                   });

      data.flux = builder.create<FrameGraphTexture>("RSM/Flux", kGBufferDesc);
      data.flux =
        builder.write(data.flux, Attachment{
                                   .index = 2,
                                   .imageAspect = rhi::ImageAspect::Color,
                                   .clearValue = ClearValue::TransparentBlack,
                                 });
    },
    [this, lightIntensity,
     batches = std::move(batches)](const ReflectiveShadowMapData &,
                                   const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, _, framebufferInfo, bindings] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      BaseGeometryPassInfo passInfo{
        .depthFormat = rhi::getDepthFormat(*framebufferInfo),
        .colorFormats = rhi::getColorFormats(*framebufferInfo),
      };
      cb.beginRendering(*framebufferInfo);
      for (const auto &batch : batches) {
        if (const auto *pipeline = _getPipeline(adjust(passInfo, batch));
            pipeline) {
          bindBatch(rc, batch);
          cb.bindPipeline(*pipeline);
          bindDescriptorSets(rc, *pipeline);
          cb.pushConstants(rhi::ShaderStages::Fragment, 16, &lightIntensity);
          drawBatch(rc, batch);
        }
      }
      endRendering(rc);
    });

  return RSMData;
}

LightPropagationVolumesData GlobalIllumination::_addRadianceInjectionPass(
  FrameGraph &fg, const FrameGraphResource sceneGridBlock,
  const ReflectiveShadowMapData &RSM, const glm::uvec3 gridSize) {
  static constexpr auto kPassName = "RadianceInjection";
  ZoneScopedN(kPassName);

  const auto LPVData = fg.addCallbackPass<LightPropagationVolumesData>(
    kPassName,
    [sceneGridBlock, &RSM, gridSize](FrameGraph::Builder &builder,
                                     LightPropagationVolumesData &data) {
      PASS_SETUP_ZONE;

      readSceneGrid(builder, sceneGridBlock, PipelineStage::VertexShader);
      read(builder, RSM, PipelineStage::VertexShader);

      const auto extent =
        rhi::Extent2D{.width = gridSize.x, .height = gridSize.y};
      const auto kSHDesc = FrameGraphTexture::Desc{
        .extent = extent,
        .depth = gridSize.z,
        .format = kSHFormat,
        .usageFlags = rhi::ImageUsage::RenderTarget | rhi::ImageUsage::Sampled,
      };
      data.r = builder.create<FrameGraphTexture>("SH/R", kSHDesc);
      data.r =
        builder.write(data.r, Attachment{
                                .index = 0,
                                .imageAspect = rhi::ImageAspect::Color,
                                .clearValue = ClearValue::TransparentBlack,
                              });

      data.g = builder.create<FrameGraphTexture>("SH/G", kSHDesc);
      data.g =
        builder.write(data.g, Attachment{
                                .index = 1,
                                .imageAspect = rhi::ImageAspect::Color,
                                .clearValue = ClearValue::TransparentBlack,
                              });

      data.b = builder.create<FrameGraphTexture>("SH/B", kSHDesc);
      data.b =
        builder.write(data.b, Attachment{
                                .index = 2,
                                .imageAspect = rhi::ImageAspect::Color,
                                .clearValue = ClearValue::TransparentBlack,
                              });
    },
    [this, gridSize](const auto &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, _, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, kPassName);

      framebufferInfo->layers = gridSize.z;

      cb.bindPipeline(m_radianceInjectionPipeline);
      bindDescriptorSets(rc, m_radianceInjectionPipeline);
      cb.beginRendering(*framebufferInfo)
        .pushConstants(rhi::ShaderStages::Vertex, 0, &kRSMResolution)
        .draw({
          .topology = rhi::PrimitiveTopology::PointList,
          .numVertices = kNumVPL,
        });
      endRendering(rc);
    });

  return LPVData;
}

LightPropagationVolumesData GlobalIllumination::_addRadiancePropagationPass(
  FrameGraph &fg, const FrameGraphResource sceneGridBlock,
  const LightPropagationVolumesData &LPV, const glm::uvec3 gridSize,
  const uint32_t iteration) {
  const auto passName = std::format("RadiancePropagation #{}", iteration);
  ZoneTransientN(__tracy_zone, passName.c_str(), true);

  const auto LPVData = fg.addCallbackPass<LightPropagationVolumesData>(
    passName,
    [sceneGridBlock, &LPV, gridSize](FrameGraph::Builder &builder,
                                     LightPropagationVolumesData &data) {
      PASS_SETUP_ZONE;

      readSceneGrid(builder, sceneGridBlock, PipelineStage::VertexShader);
      read(builder, LPV, PipelineStage::FragmentShader);

      const auto extent =
        rhi::Extent2D{.width = gridSize.x, .height = gridSize.y};
      const auto kSHDesc = FrameGraphTexture::Desc{
        .extent = extent,
        .depth = gridSize.z,
        .format = kSHFormat,
        .usageFlags = rhi::ImageUsage::RenderTarget | rhi::ImageUsage::Sampled,
      };
      data.r = builder.create<FrameGraphTexture>("SH/R", kSHDesc);
      data.r =
        builder.write(data.r, Attachment{
                                .index = 0,
                                .imageAspect = rhi::ImageAspect::Color,
                                .clearValue = ClearValue::TransparentBlack,
                              });

      data.g = builder.create<FrameGraphTexture>("SH/G", kSHDesc);
      data.g =
        builder.write(data.g, Attachment{
                                .index = 1,
                                .imageAspect = rhi::ImageAspect::Color,
                                .clearValue = ClearValue::TransparentBlack,
                              });

      data.b = builder.create<FrameGraphTexture>("SH/B", kSHDesc);
      data.b =
        builder.write(data.b, Attachment{
                                .index = 2,
                                .imageAspect = rhi::ImageAspect::Color,
                                .clearValue = ClearValue::TransparentBlack,
                              });
    },
    [this, gridSize, passName](const auto &, const FrameGraphPassResources &,
                               void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      auto &[cb, _, framebufferInfo, sets] = rc;
      RHI_GPU_ZONE(cb, passName.c_str());

      framebufferInfo->layers = gridSize.z;

      cb.bindPipeline(m_radiancePropagationPipeline);
      bindDescriptorSets(rc, m_radiancePropagationPipeline);
      cb.beginRendering(*framebufferInfo)
        .draw({
          .topology = rhi::PrimitiveTopology::PointList,
          .numVertices = kNumVPL,
        });
      endRendering(rc);
    });

  return LPVData;
}

rhi::GraphicsPipeline GlobalIllumination::_createPipeline(
  const BaseGeometryPassInfo &passInfo) const {
  assert(passInfo.vertexFormat && passInfo.material);

  auto &rd = getRenderDevice();

  const auto &material = *passInfo.material;
  const auto [vertCode, fragCode] =
    buildShaderCode(rd, passInfo.vertexFormat, material);

  return rhi::GraphicsPipeline::Builder{}
    .setDepthFormat(passInfo.depthFormat)
    .setColorFormats(passInfo.colorFormats)
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
      .cullMode = getSurface(material).cullMode,
    })
    .setBlending(0, {.enabled = false})
    .setBlending(1, {.enabled = false})
    .setBlending(2, {.enabled = false})
    .build(rd);
}

} // namespace gfx
