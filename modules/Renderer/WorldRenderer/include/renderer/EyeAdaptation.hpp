#pragma once

#include "fg/Fwd.hpp"
#include "rhi/Extent2D.hpp"
#include "rhi/ComputePipeline.hpp"
#include "Technique.hpp"

#include "robin_hood.h"

namespace rhi {
class Texture;
}

namespace gfx {

struct AdaptiveExposure;

class EyeAdaptation final : public Technique {
public:
  explicit EyeAdaptation(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  using ID = uint64_t;

  void compute(FrameGraph &, FrameGraphBlackboard &, const AdaptiveExposure &,
               const ID uid, const float deltaTime);

private:
  class HistogramBuilder final {
  public:
    explicit HistogramBuilder(rhi::RenderDevice &);

    [[nodiscard]] FrameGraphResource
    buildHistogram(FrameGraph &, const FrameGraphResource sceneColor,
                   const float minLogLuminance, const float logLuminanceRange);

  private:
    rhi::ComputePipeline m_pipeline;
  };

  class AverageLuminance final {
  public:
    explicit AverageLuminance(rhi::RenderDevice &);

    [[nodiscard]] FrameGraphResource
    calculateAverageLuminance(FrameGraph &, const FrameGraphResource histogram,
                              const float minLogLuminance,
                              const float logLuminanceRange,
                              const rhi::Extent2D dimensions, const float tau,
                              const ID, const float deltaTime);

  private:
    [[nodiscard]] rhi::Texture *_getAverageLuminanceTexture(const ID);

  private:
    rhi::RenderDevice &m_renderDevice;

    rhi::ComputePipeline m_pipeline;
    using TextureCache =
      robin_hood::unordered_map<ID, std::unique_ptr<rhi::Texture>>;
    TextureCache m_textureCache;
  };

private:
  HistogramBuilder m_histogramBuilder;
  AverageLuminance m_averageLuminance;
};

} // namespace gfx
