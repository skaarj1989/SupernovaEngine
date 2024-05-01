#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"

namespace gfx {

class SSR final : public rhi::RenderPass<SSR>, public Technique {
  friend class BasePass;

public:
  explicit SSR(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  [[nodiscard]] FrameGraphResource addPass(FrameGraph &,
                                           FrameGraphBlackboard &);

private:
  rhi::GraphicsPipeline
  _createPipeline(const rhi::PixelFormat colorFormat) const;
};

} // namespace gfx
