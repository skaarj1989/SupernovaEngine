#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"

namespace gfx {

struct WeightedBlendedData;

class TransparencyCompositionPass final
    : public rhi::RenderPass<TransparencyCompositionPass> {
  friend class BasePass;

public:
  explicit TransparencyCompositionPass(rhi::RenderDevice &);

  [[nodiscard]] FrameGraphResource
  addPass(FrameGraph &, const WeightedBlendedData &, FrameGraphResource target);

private:
  rhi::GraphicsPipeline _createPipeline(rhi::PixelFormat colorFormat) const;
};

} // namespace gfx
