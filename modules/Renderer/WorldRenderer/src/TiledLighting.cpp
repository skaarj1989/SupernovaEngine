#include "renderer/TiledLighting.hpp"
#include "renderer/Blit.hpp"

#include "FrameGraphCommon.hpp"
#include "renderer/FrameGraphBuffer.hpp"
#include "renderer/FrameGraphTexture.hpp"
#include "FrameGraphResourceAccess.hpp"

#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/GBuffer.hpp"
#include "FrameGraphData/Lights.hpp"
#include "FrameGraphData/LightCulling.hpp"

#include "ShaderCodeBuilder.hpp"
#include "RenderContext.hpp"

namespace gfx {

namespace {

constexpr auto kUseDebugOutput = true;

struct GPUFrustumTile {
  std::array<glm::vec4, 4> planes;
};
static_assert(sizeof(GPUFrustumTile) == 64);

} // namespace

//
// TiledLighting class:
//

TiledLighting::TiledLighting(rhi::RenderDevice &rd)
    : m_frustumBuilder{rd}, m_lightCuller{rd} {}

uint32_t TiledLighting::count(PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn)
           ? m_frustumBuilder.count() + m_lightCuller.count()
           : 0;
}
void TiledLighting::clear(PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) {
    m_frustumBuilder.clear();
    m_lightCuller.clear();
  }
}

void TiledLighting::cullLights(FrameGraph &fg, FrameGraphBlackboard &blackboard,
                               TileSize tileSize) {
  ZoneScoped;

  const auto depthBuffer = blackboard.get<GBufferData>().depth;
  const auto extent = fg.getDescriptor<FrameGraphTexture>(depthBuffer).extent;
  const auto gridSize = rhi::calcNumWorkGroups(glm::uvec2{extent}, tileSize);

  const PassInfo passInfo{
    .tileSize = tileSize,
    .numFrustums = gridSize.x * gridSize.y,
    .gridSize = gridSize,
  };
  const auto gridFrustums =
    m_frustumBuilder.buildFrustums(fg, blackboard, passInfo);
  m_lightCuller.cullLights(fg, blackboard, gridFrustums, passInfo);
}

FrameGraphResource
TiledLighting::addDebugOverlay(FrameGraph &fg, FrameGraphBlackboard &blackboard,
                               Blit &blit, FrameGraphResource target) {
  ZoneScoped;

  const auto *d = blackboard.try_get<LightCullingData>();
  return d && d->debugMap ? blit.mix(fg, target, *d->debugMap) : target;
}

//
// FrustumBuilder class:
//

TiledLighting::FrustumBuilder::FrustumBuilder(rhi::RenderDevice &rd)
    : rhi::ComputePass<FrustumBuilder>{rd} {}

FrameGraphResource TiledLighting::FrustumBuilder::buildFrustums(
  FrameGraph &fg, FrameGraphBlackboard &blackboard, const PassInfo &passInfo) {
  ZoneScoped;

  constexpr auto kPassName = "BuildFrustums";

  struct FrustumsData {
    FrameGraphResource gridFrustums;
  };
  const auto [gridFrustums] = fg.addCallbackPass<FrustumsData>(
    kPassName,
    [&blackboard, &passInfo](FrameGraph::Builder &builder, FrustumsData &data) {
      read(builder, blackboard.get<CameraData>(), PipelineStage::ComputeShader);

      data.gridFrustums = builder.create<FrameGraphBuffer>(
        "GridFrustums", {
                          .type = BufferType::StorageBuffer,
                          .stride = sizeof(GPUFrustumTile),
                          .capacity = passInfo.numFrustums,
                        });
      data.gridFrustums = builder.write(
        data.gridFrustums, BindingInfo{
                             .location = {.set = 2, .binding = 0},
                             .pipelineStage = PipelineStage::ComputeShader,
                           });
    },
    [this, passInfo](const FrustumsData &, const FrameGraphPassResources &,
                     void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, kPassName)

      auto &[cb, _, sets] = rc;
      const auto *pipeline = _getPipeline(passInfo.tileSize);
      if (pipeline) {
        cb.bindPipeline(*pipeline);
        bindDescriptorSets(rc, *pipeline);
        cb.pushConstants(rhi::ShaderStages::Compute, 0, &passInfo.gridSize);
        cb.dispatch(
          {rhi::calcNumWorkGroups(passInfo.gridSize, passInfo.tileSize), 1u});
      }
      sets.clear();
    });

  return gridFrustums;
}

rhi::ComputePipeline
TiledLighting::FrustumBuilder::_createPipeline(TileSize tileSize) const {
  return getRenderDevice().createComputePipeline(
    ShaderCodeBuilder{}
      .addDefine("TILE_SIZE", tileSize)
      .buildFromFile("LightCulling/BuildFrustums.comp"));
}

//
// LightCuller class:
//

TiledLighting::LightCuller::LightCuller(rhi::RenderDevice &rd)
    : rhi::ComputePass<LightCuller>{rd} {}

void TiledLighting::LightCuller::cullLights(FrameGraph &fg,
                                            FrameGraphBlackboard &blackboard,
                                            FrameGraphResource gridFrustums,
                                            const PassInfo &passInfo) {
  ZoneScoped;

  constexpr auto kPassName = "CullLights";

  struct Data : LightCullingData {
    // Internal use only, can be sliced away safely.
    FrameGraphResource lightsCounter;
  };
  blackboard.add<LightCullingData>() = fg.addCallbackPass<Data>(
    kPassName,
    [&fg, &blackboard, gridFrustums, &passInfo](FrameGraph::Builder &builder,
                                                Data &data) {
      data.tileSize = passInfo.tileSize;

      read(builder, blackboard.get<CameraData>(), PipelineStage::ComputeShader);
      read(builder, blackboard.get<LightsData>(), PipelineStage::ComputeShader);
      builder.read(gridFrustums,
                   BindingInfo{
                     .location = {.set = 2, .binding = 1},
                     .pipelineStage = PipelineStage::ComputeShader,
                   });

      const auto depthBuffer = blackboard.get<GBufferData>().depth;
      builder.read(depthBuffer,
                   TextureRead{
                     .binding =
                       {
                         .location = {.set = 2, .binding = 0},
                         .pipelineStage = PipelineStage::ComputeShader,
                       },
                     .type = TextureRead::Type::SampledImage,
                   });

      data.lightsCounter = builder.create<FrameGraphBuffer>(
        "LightsCounter", {
                           .type = BufferType::StorageBuffer,
                           .stride = sizeof(glm::uvec2),
                           .capacity = 1,
                         });
      data.lightsCounter = builder.write(
        data.lightsCounter, BindingInfo{
                              .location = {.set = 2, .binding = 2},
                              .pipelineStage = PipelineStage::ComputeShader,
                            });

      data.lightGrid = builder.create<FrameGraphTexture>(
        "LightGrid", {
                       .extent = {passInfo.gridSize.x, passInfo.gridSize.y},
                       .format = rhi::PixelFormat::RGBA32UI,
                       .usageFlags = rhi::ImageUsage::Storage,
                     });
      data.lightGrid = builder.write(
        data.lightGrid, BindingInfo{
                          .location = {.set = 2, .binding = 3},
                          .pipelineStage = PipelineStage::ComputeShader,
                        });

      const auto averageOverlappingLightsPerTile =
        passInfo.tileSize * passInfo.tileSize;
      data.lightIndices = builder.create<FrameGraphBuffer>(
        "LightIndices",
        {
          .type = BufferType::StorageBuffer,
          .stride = sizeof(glm::uvec2),
          .capacity = passInfo.numFrustums * averageOverlappingLightsPerTile,
        });
      data.lightIndices = builder.write(
        data.lightIndices, BindingInfo{
                             .location = {.set = 2, .binding = 4},
                             .pipelineStage = PipelineStage::ComputeShader,
                           });

      if constexpr (kUseDebugOutput) {
        const auto extent =
          fg.getDescriptor<FrameGraphTexture>(depthBuffer).extent;

        data.debugMap = builder.create<FrameGraphTexture>(
          "DebugMap",
          {
            .extent = extent,
            .format = rhi::PixelFormat::RGBA8_UNorm,
            .usageFlags = rhi::ImageUsage::Storage | rhi::ImageUsage::Sampled,
          });
        data.debugMap = builder.write(
          *data.debugMap, BindingInfo{
                            .location = {.set = 2, .binding = 5},
                            .pipelineStage = PipelineStage::ComputeShader,
                          });
      }
    },
    [this, passInfo](const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      ZONE(rc, kPassName)

      auto &[cb, _, sets] = rc;
      const auto *pipeline = _getPipeline(passInfo.tileSize);
      if (pipeline) {
        cb.bindPipeline(*pipeline);
        bindDescriptorSets(rc, *pipeline);
        cb.dispatch({passInfo.gridSize, 1u});
      }
      sets.clear();
    });
}

rhi::ComputePipeline
TiledLighting::LightCuller::_createPipeline(TileSize tileSize) const {
  return getRenderDevice().createComputePipeline(
    ShaderCodeBuilder{}
      .addDefine("TILE_SIZE", tileSize)
      // 0 = Disabled, 1 = Heatmap, 2 = Depth
      .addDefine("_DEBUG_OUTPUT", kUseDebugOutput ? 1 : 0)
      .buildFromFile("LightCulling/CullLights.comp"));
}

} // namespace gfx
