#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "OutputMode.hpp"

namespace gfx {

struct CommonSamplers;

class FinalPass final : public rhi::RenderPass<FinalPass>, public Technique {
  friend class BasePass;

public:
  FinalPass(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  FrameGraphResource compose(FrameGraph &, const FrameGraphBlackboard &,
                             OutputMode, FrameGraphResource target);

private:
  enum class Mode { Final, Pattern };

  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(rhi::PixelFormat colorFormat, Mode) const;

private:
  const CommonSamplers &m_samplers;
};

} // namespace gfx
