#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"

namespace gfx {

class Downsampler final : public rhi::RenderPass<Downsampler> {
  friend class BasePass;

public:
  explicit Downsampler(rhi::RenderDevice &);

  [[nodiscard]] FrameGraphResource
  addPass(FrameGraph &, const FrameGraphResource input, const uint32_t level);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const rhi::PixelFormat colorFormat) const;
};

} // namespace gfx
