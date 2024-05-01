#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"

namespace gfx {

class Blit final : public rhi::RenderPass<Blit>, public Technique {
  friend class BasePass;

public:
  explicit Blit(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  // Mixes a and b (by alpha of b).
  [[nodiscard]] FrameGraphResource mix(FrameGraph &, const FrameGraphResource a,
                                       const FrameGraphResource b);

  // Adds a given source color to the target texture (additive blending).
  [[nodiscard]] FrameGraphResource addColor(FrameGraph &,
                                            const FrameGraphResource target,
                                            const FrameGraphResource source);

  // Same as addColor, but for multiple sources (in a single pass).
  [[nodiscard]] FrameGraphResource
  merge(FrameGraph &, FrameGraphResource target,
        const std::vector<FrameGraphResource> &);

  struct MixPassInfo {
    rhi::PixelFormat colorFormat;
  };
  struct MergePassInfo {
    rhi::PixelFormat colorFormat;
    std::size_t numTextures;
  };

private:
  rhi::GraphicsPipeline _createPipeline(const MixPassInfo &) const;
  rhi::GraphicsPipeline _createPipeline(const MergePassInfo &) const;
};

} // namespace gfx
