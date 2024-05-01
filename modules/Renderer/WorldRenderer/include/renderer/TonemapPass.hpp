#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "Tonemap.hpp"

namespace gfx {

class TonemapPass final : public rhi::RenderPass<TonemapPass>,
                          public Technique {
  friend class BasePass;

public:
  explicit TonemapPass(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  [[nodiscard]] FrameGraphResource addPass(FrameGraph &, FrameGraphBlackboard &,
                                           const Tonemap, const float exposure,
                                           const float bloomStrength);

private:
  rhi::GraphicsPipeline _createPipeline(const rhi::PixelFormat colorFormat,
                                        const bool autoExposure,
                                        const bool bloom) const;
};

} // namespace gfx
