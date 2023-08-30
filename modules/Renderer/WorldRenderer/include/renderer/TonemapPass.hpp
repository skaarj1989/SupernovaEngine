#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "Tonemap.hpp"

namespace gfx {

struct CommonSamplers;

class TonemapPass final : public rhi::RenderPass<TonemapPass>,
                          public Technique {
  friend class BasePass;

public:
  TonemapPass(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  [[nodiscard]] FrameGraphResource addPass(FrameGraph &, FrameGraphBlackboard &,
                                           Tonemap, float exposure,
                                           float bloomStrength);

private:
  rhi::GraphicsPipeline _createPipeline(rhi::PixelFormat colorFormat,
                                        bool autoExposure, bool bloom) const;

private:
  const CommonSamplers &m_samplers;
};

} // namespace gfx
