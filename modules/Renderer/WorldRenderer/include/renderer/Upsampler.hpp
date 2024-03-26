#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"

namespace gfx {

struct CommonSamplers;

class Upsampler final : public rhi::RenderPass<Upsampler> {
  friend class BasePass;

public:
  Upsampler(rhi::RenderDevice &, const CommonSamplers &);

  [[nodiscard]] FrameGraphResource
  addPass(FrameGraph &, const FrameGraphResource input, const float radius);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const rhi::PixelFormat colorFormat) const;

private:
  const CommonSamplers &m_samplers;
};

} // namespace gfx
