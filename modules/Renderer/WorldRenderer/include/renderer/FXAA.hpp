#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"

namespace gfx {

class FXAA final : public rhi::RenderPass<FXAA>, public Technique {
  friend class BasePass;

public:
  explicit FXAA(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  [[nodiscard]] FrameGraphResource addPass(FrameGraph &,
                                           const FrameGraphBlackboard &,
                                           const FrameGraphResource input);

private:
  rhi::GraphicsPipeline
  _createPipeline(const rhi::PixelFormat colorFormat) const;
};

} // namespace gfx
