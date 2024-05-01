#include "renderer/EyeAdaptation.hpp"
#include "rhi/RenderDevice.hpp"

#include "renderer/AdaptiveExposure.hpp"

#include "fg/FrameGraph.hpp"
#include "fg/Blackboard.hpp"
#include "FrameGraphResourceAccess.hpp"
#include "renderer/FrameGraphBuffer.hpp"
#include "renderer/FrameGraphTexture.hpp"
#include "FrameGraphImport.hpp"

#include "FrameGraphData/SceneColor.hpp"
#include "FrameGraphData/AverageLuminance.hpp"

#include "RenderContext.hpp"
#include "ShaderCodeBuilder.hpp"

#include "glm/common.hpp"      // clamp
#include "glm/exponential.hpp" // exp

namespace gfx {

namespace {

constexpr auto kTileSize = 16u;
constexpr auto kNumHistogramBins = 256u;

[[nodiscard]] auto createAverageLuminanceTexture(rhi::RenderDevice &rd) {
  ZoneScopedN("CreateAvgLuminanceTexture");

  auto texture =
    rhi::Texture::Builder{}
      .setExtent({1, 1})
      .setPixelFormat(rhi::PixelFormat::R16F)
      .setUsageFlags(rhi::ImageUsage::TransferDst | rhi::ImageUsage::Storage)
      .setupOptimalSampler(false)
      .build(rd);

  rd.execute([&texture](rhi::CommandBuffer &cb) {
    cb.getBarrierBuilder().imageBarrier(
      {
        .image = texture,
        .newLayout = rhi::ImageLayout::General,
      },
      {
        .stageMask = rhi::PipelineStages::Transfer,
        .accessMask = rhi::Access::TransferWrite,
      });
    // UNASSIGNED-BestPractices-ClearColor-NotCompressed
    cb.clear(texture, glm::vec4{0.0f}); // Use 0 or 1.
  });

  return texture;
}

[[nodiscard]] auto createHistogram(FrameGraph &fg) {
  static constexpr auto kPassName = "CreateHistogram";
  ZoneScopedN(kPassName);

  struct Data {
    FrameGraphResource histogram;
  };
  auto [histogram] = fg.addCallbackPass<Data>(
    kPassName,
    [](FrameGraph::Builder &builder, Data &data) {
      PASS_SETUP_ZONE;

      data.histogram = builder.create<FrameGraphBuffer>(
        "Histogram", {
                       .type = BufferType::StorageBuffer,
                       .stride = sizeof(uint32_t),
                       .capacity = kNumHistogramBins,
                     });
      data.histogram = builder.write(
        data.histogram, BindingInfo{.pipelineStage = PipelineStage::Transfer});
    },
    [](const Data &data, FrameGraphPassResources &resources, void *ctx) {
      auto &cb = static_cast<RenderContext *>(ctx)->commandBuffer;
      RHI_GPU_ZONE(cb, kPassName);
      cb.clear(*resources.get<FrameGraphBuffer>(data.histogram).buffer);
    });

  return histogram;
}

} // namespace

//
// EyeAdaptation class:
//

EyeAdaptation::EyeAdaptation(rhi::RenderDevice &rd)
    : m_histogramBuilder{rd}, m_averageLuminance{rd} {}

uint32_t EyeAdaptation::count(const PipelineGroups flags) const {
  return bool(flags & PipelineGroups::BuiltIn) ? 2 : 0;
}
void EyeAdaptation::clear(const PipelineGroups) {
  /* Pipeline rebuilding unsupported. */
}

void EyeAdaptation::compute(FrameGraph &fg, FrameGraphBlackboard &blackboard,
                            const AdaptiveExposure &adaptiveExposure,
                            const ID uid, const float deltaTime) {
  ZoneScopedN("EyeAdaptation");

  const auto sceneColor = blackboard.get<SceneColorData>().HDR;

  const auto logLuminanceRange =
    adaptiveExposure.maxLogLuminance - adaptiveExposure.minLogLuminance;

  const auto histogram = m_histogramBuilder.buildHistogram(
    fg, sceneColor, adaptiveExposure.minLogLuminance, logLuminanceRange);

  const auto extent = fg.getDescriptor<FrameGraphTexture>(sceneColor).extent;
  blackboard.add<AverageLuminanceData>().averageLuminance =
    m_averageLuminance.calculateAverageLuminance(
      fg, histogram, adaptiveExposure.minLogLuminance, logLuminanceRange,
      extent, adaptiveExposure.tau, uid, deltaTime);
}

//
// HistogramBuilder class:
//

EyeAdaptation::HistogramBuilder::HistogramBuilder(rhi::RenderDevice &rd) {
  m_pipeline = rd.createComputePipeline(
    ShaderCodeBuilder{}
      .addDefine("TILE_SIZE", kTileSize)
      .addDefine("NUM_HISTOGRAM_BINS", kNumHistogramBins)
      .buildFromFile("BuildHistogram.comp"));
}

FrameGraphResource EyeAdaptation::HistogramBuilder::buildHistogram(
  FrameGraph &fg, const FrameGraphResource sceneColor,
  const float minLogLuminance, const float logLuminanceRange) {
  static constexpr auto kPassName = "BuildHistogram";
  ZoneScopedN(kPassName);

  // TransientResources system does not guarantee that acquired buffer will be
  // cleared, hence the separate clear pass.
  auto histogram = createHistogram(fg);

  fg.addCallbackPass(
    kPassName,
    [sceneColor, &histogram](FrameGraph::Builder &builder, auto &) {
      PASS_SETUP_ZONE;

      builder.read(sceneColor,
                   TextureRead{
                     .binding =
                       {
                         .location = {.set = 0, .binding = 0},
                         .pipelineStage = PipelineStage::ComputeShader,
                       },
                     .type = TextureRead::Type::SampledImage,
                     .imageAspect = rhi::ImageAspect::Color,
                   });

      histogram = builder.write(histogram,
                                BindingInfo{
                                  .location = {.set = 0, .binding = 1},
                                  .pipelineStage = PipelineStage::ComputeShader,
                                });
    },
    [this, sceneColor, minLogLuminance, logLuminanceRange](
      const auto &, const FrameGraphPassResources &resources, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      RHI_GPU_ZONE(rc.commandBuffer, kPassName);

      struct Uniforms {
        float minLogLuminance;
        float oneOverLuminanceRange;
        glm::uvec2 extent;
      };
      const auto extent =
        resources.getDescriptor<FrameGraphTexture>(sceneColor).extent;
      const Uniforms uniforms{
        .minLogLuminance = minLogLuminance,
        .oneOverLuminanceRange = 1.0f / logLuminanceRange,
        .extent = glm::uvec2{extent},
      };
      rc.commandBuffer.bindPipeline(m_pipeline);
      bindDescriptorSets(rc, m_pipeline);
      rc.commandBuffer.pushConstants(rhi::ShaderStages::Compute, 0, &uniforms)
        .dispatch({rhi::calcNumWorkGroups(glm::uvec2{extent}, kTileSize), 1u});

      rc.resourceSet.clear();
    });

  return histogram;
}

//
// AverageLuminance class:
//

EyeAdaptation::AverageLuminance::AverageLuminance(rhi::RenderDevice &rd)
    : m_renderDevice{rd} {
  m_pipeline = m_renderDevice.createComputePipeline(
    ShaderCodeBuilder{}
      .addDefine("NUM_HISTOGRAM_BINS", kNumHistogramBins)
      .buildFromFile("AverageLuminance.comp"));
}

FrameGraphResource EyeAdaptation::AverageLuminance::calculateAverageLuminance(
  FrameGraph &fg, const FrameGraphResource histogram,
  const float minLogLuminance, const float logLuminanceRange,
  const rhi::Extent2D dimensions, const float tau, const ID uid,
  const float deltaTime) {
  static constexpr auto kPassName = "AverageLuminance";
  ZoneScopedN(kPassName);

  struct Data {
    FrameGraphResource averageLuminance;
  };
  const auto [averageLuminance] = fg.addCallbackPass<Data>(
    kPassName,
    [this, &fg, histogram, uid](FrameGraph::Builder &builder, Data &data) {
      PASS_SETUP_ZONE;

      builder.read(histogram, BindingInfo{
                                .location = {.set = 0, .binding = 1},
                                .pipelineStage = PipelineStage::ComputeShader,
                              });

      auto avgLuminance = _getAverageLuminanceTexture(uid);
      data.averageLuminance = importTexture(
        fg, std::format("AvgLuminance<BR/>[uid: {}]", uid), avgLuminance);
      data.averageLuminance =
        builder.write(data.averageLuminance,
                      ImageWrite{
                        .binding =
                          {
                            .location = {.set = 0, .binding = 2},
                            .pipelineStage = PipelineStage::ComputeShader,
                          },
                        .imageAspect = rhi::ImageAspect::Color,
                      });
    },
    [this, minLogLuminance, logLuminanceRange, dimensions, tau,
     deltaTime](const Data &, const FrameGraphPassResources &, void *ctx) {
      auto &rc = *static_cast<RenderContext *>(ctx);
      RHI_GPU_ZONE(rc.commandBuffer, kPassName);

      struct Uniforms {
        float minLogLuminance;
        float logLuminanceRange;
        float timeCoeff;
        uint32_t numPixels;
      };
      const Uniforms uniforms{
        .minLogLuminance = minLogLuminance,
        .logLuminanceRange = logLuminanceRange,
        .timeCoeff = glm::clamp(1.0f - glm::exp(-deltaTime * tau), 0.0f, 1.0f),
        .numPixels = dimensions.width * dimensions.height,
      };
      rc.commandBuffer.bindPipeline(m_pipeline);
      bindDescriptorSets(rc, m_pipeline);
      rc.commandBuffer.pushConstants(rhi::ShaderStages::Compute, 0, &uniforms)
        .dispatch(glm::uvec3{1u});

      rc.resourceSet.clear();
    });

  return averageLuminance;
}

rhi::Texture *
EyeAdaptation::AverageLuminance::_getAverageLuminanceTexture(const ID uid) {
  if (const auto it = m_textureCache.find(uid); it != m_textureCache.cend())
    return it->second.get();

  const auto &[it, _] = m_textureCache.emplace(
    uid, std::make_unique<rhi::Texture>(
           createAverageLuminanceTexture(m_renderDevice)));
  return it->second.get();
}

} // namespace gfx
