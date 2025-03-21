#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "rhi/Texture.hpp"
#include "rhi/UniformBuffer.hpp"
#include "Technique.hpp"

namespace gfx {

struct SSAOSettings;

class Blur;

class SSAO final : public rhi::RenderPass<SSAO>, public Technique {
  friend class BasePass;

public:
  explicit SSAO(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  using Settings = SSAOSettings;

  void addPass(FrameGraph &, FrameGraphBlackboard &, Blur &, const Settings &);

private:
  rhi::GraphicsPipeline
  _createPipeline(const rhi::PixelFormat colorFormat) const;

private:
  rhi::Texture m_noise;
  rhi::UniformBuffer m_kernelBuffer;
};

} // namespace gfx
