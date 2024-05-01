#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"

namespace gfx {

class Upsampler final : public rhi::RenderPass<Upsampler> {
  friend class BasePass;

public:
  explicit Upsampler(rhi::RenderDevice &);

  [[nodiscard]] FrameGraphResource
  addPass(FrameGraph &, const FrameGraphResource input, const float radius);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const rhi::PixelFormat colorFormat) const;
};

} // namespace gfx
