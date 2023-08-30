#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "SSAOSettings.hpp"

namespace gfx {

class Blur;
struct CommonSamplers;

class SSAO final : public rhi::RenderPass<SSAO>, public Technique {
  friend class BasePass;

public:
  SSAO(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  using Settings = SSAOSettings;

  void addPass(FrameGraph &, FrameGraphBlackboard &, Blur &, const Settings &);

private:
  rhi::GraphicsPipeline _createPipeline(rhi::PixelFormat colorFormat) const;

private:
  rhi::Texture m_noise;
  rhi::UniformBuffer m_kernelBuffer;

  const CommonSamplers &m_samplers;
};

} // namespace gfx
