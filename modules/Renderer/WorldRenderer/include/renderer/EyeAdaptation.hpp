#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderDevice.hpp"
#include "Technique.hpp"
#include "AdaptiveExposure.hpp"

namespace gfx {

class EyeAdaptation final : public Technique {
public:
  explicit EyeAdaptation(rhi::RenderDevice &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  void compute(FrameGraph &, FrameGraphBlackboard &, const AdaptiveExposure &,
               uint64_t uid, float deltaTime);

private:
  class HistogramBuilder final {
  public:
    explicit HistogramBuilder(rhi::RenderDevice &);

    [[nodiscard]] FrameGraphResource
    buildHistogram(FrameGraph &, FrameGraphResource sceneColor,
                   float minLogLuminance, float logLuminanceRange);

  private:
    rhi::ComputePipeline m_pipeline;
  };

  class AverageLuminance final {
  public:
    explicit AverageLuminance(rhi::RenderDevice &);

    [[nodiscard]] FrameGraphResource
    calculateAverageLuminance(FrameGraph &, FrameGraphResource histogram,
                              float minLogLuminance, float logLuminanceRange,
                              rhi::Extent2D dimensions, float tau, uint64_t uid,
                              float deltaTime);

  private:
    [[nodiscard]] rhi::Texture *_getAverageLuminanceTexture(uint64_t);

  private:
    rhi::RenderDevice &m_renderDevice;

    rhi::ComputePipeline m_pipeline;
    using TextureCache =
      robin_hood::unordered_map<uint64_t, std::unique_ptr<rhi::Texture>>;
    TextureCache m_textureCache;
  };

private:
  HistogramBuilder m_histogramBuilder;
  AverageLuminance m_averageLuminance;
};

} // namespace gfx
