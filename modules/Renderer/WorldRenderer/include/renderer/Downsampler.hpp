#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"

namespace gfx {

struct CommonSamplers;

class Downsampler final : public rhi::RenderPass<Downsampler> {
  friend class BasePass;

public:
  Downsampler(rhi::RenderDevice &, const CommonSamplers &);

  [[nodiscard]] FrameGraphResource
  addPass(FrameGraph &, FrameGraphResource input, uint32_t level);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(rhi::PixelFormat colorFormat) const;

private:
  const CommonSamplers &m_samplers;
};

} // namespace gfx
