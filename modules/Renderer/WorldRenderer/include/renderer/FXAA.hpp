#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"

namespace gfx {

struct CommonSamplers;

class FXAA final : public rhi::RenderPass<FXAA>, public Technique {
  friend class BasePass;

public:
  FXAA(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  [[nodiscard]] FrameGraphResource
  addPass(FrameGraph &, const FrameGraphBlackboard &, FrameGraphResource input);

private:
  rhi::GraphicsPipeline _createPipeline(rhi::PixelFormat colorFormat) const;

private:
  const CommonSamplers &m_samplers;
};

} // namespace gfx
