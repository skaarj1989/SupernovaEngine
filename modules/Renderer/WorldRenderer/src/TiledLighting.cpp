#include "renderer/TiledLighting.hpp"
#include "rhi/RenderDevice.hpp"

#include "renderer/Blit.hpp"

#include "FrameGraphResourceAccess.hpp"
#include "FrameGraphCommon.hpp"
#include "renderer/FrameGraphBuffer.hpp"
#include "renderer/FrameGraphTexture.hpp"

#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/GBuffer.hpp"
#include "FrameGraphData/Lights.hpp"
#include "FrameGraphData/LightCulling.hpp"

#include "RenderContext.hpp"
#include "ShaderCodeBuilder.hpp"

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

uint32_t TiledLighting::count(const PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn)
           ? m_frustumBuilder.count() + m_lightCuller.count()
           : 0;
}
void TiledLighting::clear(const PipelineGroups flags) {
  if (bool(flags & PipelineGroups::BuiltIn)) {
    m_frustumBuilder.clear();
    m_lightCuller.clear();
  }
}

void TiledLighting::cullLights(FrameGraph &fg, FrameGraphBlackboard &blackboard,
                               const TileSize tileSize) {
  ZoneScopedN("TiledLighting");

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
TiledLighting::addDebugOverlay(FrameGraph &fg,
                               const FrameGraphBlackboard &blackboard,
                               Blit &blit, const FrameGraphResource target) {
  ZoneScopedN("TiledLighting::Debug");

  const auto *d = blackboard.try_get<LightCullingData>();
  return d && d->debugMap ? blit.mix(fg, target, *d->debugMap) : target;
}

//
// FrustumBuilder class:
//

TiledLighting::FrustumBuilder::FrustumBuilder(rhi::RenderDevice &rd)
    : rhi::ComputePass<FrustumBuilder>{rd} {}

FrameGraphResource TiledLighting::FrustumBuilder::buildFrustums(
  FrameGraph &fg, const FrameGraphBlackboard &blackboard,
  const PassInfo &passInfo) {
  static constexpr auto kPassName = "BuildFrustums";
  ZoneScopedN(kPassName);

  struct FrustumsData {
    FrameGraphResource gridFrustums;
  };
  const auto [gridFrustums] = fg.addCallbackPass<FrustumsData>(
    kPassName,
    [&blackboard, &passInfo](FrameGraph::Builder &builder, FrustumsData &data) {
      PASS_SETUP_ZONE;

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
      RHI_GPU_ZONE(rc.commandBuffer, kPassName);

      const auto *pipeline = _getPipeline(passInfo.tileSize);
      if (pipeline) {
        rc.commandBuffer.bindPipeline(*pipeline);
        bindDescriptorSets(rc, *pipeline);
        rc.commandBuffer
          .pushConstants(rhi::ShaderStages::Compute, 0, &passInfo.gridSize)
          .dispatch(
            {rhi::calcNumWorkGroups(passInfo.gridSize, passInfo.tileSize), 1u});
      }
      rc.resourceSet.clear();
    });

  return gridFrustums;
}

rhi::ComputePipeline
TiledLighting::FrustumBuilder::_createPipeline(const TileSize tileSize) const {
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

void TiledLighting::LightCuller::cullLights(
  FrameGraph &fg, FrameGraphBlackboard &blackboard,
  const FrameGraphResource gridFrustums, const PassInfo &passInfo) {
  static constexpr auto kPassName = "CullLights";
  ZoneScopedN(kPassName);

  struct Data : LightCullingData {
    // Internal use only, can be sliced away safely.
    FrameGraphResource lightsCounter;
  };
  blackboard.add<LightCullingData>() = fg.addCallbackPass<Data>(
    kPassName,
    [&fg, &blackboard, gridFrustums, &passInfo](FrameGraph::Builder &builder,
                                                Data &data) {
      PASS_SETUP_ZONE;

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
                     .imageAspect = rhi::ImageAspect::Depth,
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
        data.lightGrid, ImageWrite{
                          .binding =
                            {
                              .location = {.set = 2, .binding = 3},
                              .pipelineStage = PipelineStage::ComputeShader,
                            },
                          .imageAspect = rhi::ImageAspect::Color,
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
          *data.debugMap, ImageWrite{
                            .binding =
                              {
                                .location = {.set = 2, .binding = 5},
                                .pipelineStage = PipelineStage::ComputeShader,
                              },
                            .imageAspect = rhi::ImageAspect::Color,
                          });
      }
    },
    [this, passInfo](const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      RHI_GPU_ZONE(rc.commandBuffer, kPassName);

      const auto *pipeline = _getPipeline(passInfo.tileSize);
      if (pipeline) {
        rc.commandBuffer.bindPipeline(*pipeline);
        bindDescriptorSets(rc, *pipeline);
        rc.commandBuffer.dispatch({passInfo.gridSize, 1u});
      }
      rc.resourceSet.clear();
    });
}

rhi::ComputePipeline
TiledLighting::LightCuller::_createPipeline(const TileSize tileSize) const {
  return getRenderDevice().createComputePipeline(
    ShaderCodeBuilder{}
      .addDefine("TILE_SIZE", tileSize)
      // 0 = Disabled, 1 = Heatmap, 2 = Depth
      .addDefine("_DEBUG_OUTPUT", kUseDebugOutput ? 1 : 0)
      .buildFromFile("LightCulling/CullLights.comp"));
}

} // namespace gfx
