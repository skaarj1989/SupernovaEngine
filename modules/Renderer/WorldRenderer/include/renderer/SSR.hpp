#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"

namespace gfx {

struct CommonSamplers;

class SSR final : public rhi::RenderPass<SSR>, public Technique {
  friend class BasePass;

public:
  SSR(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  [[nodiscard]] FrameGraphResource addPass(FrameGraph &,
                                           FrameGraphBlackboard &);

private:
  rhi::GraphicsPipeline
  _createPipeline(const rhi::PixelFormat colorFormat) const;

private:
  const CommonSamplers &m_samplers;
};

} // namespace gfx
