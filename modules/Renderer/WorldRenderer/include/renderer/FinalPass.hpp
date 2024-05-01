#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "OutputMode.hpp"

namespace gfx {

class FinalPass final : public rhi::RenderPass<FinalPass>, public Technique {
  friend class BasePass;

public:
  explicit FinalPass(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  FrameGraphResource compose(FrameGraph &, const FrameGraphBlackboard &,
                             const OutputMode, FrameGraphResource target);

private:
  enum class Mode { Final, Pattern };

  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const rhi::PixelFormat colorFormat, const Mode) const;
};

} // namespace gfx
