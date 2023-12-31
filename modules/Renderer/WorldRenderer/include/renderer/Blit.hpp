#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"

namespace gfx {

struct CommonSamplers;

class Blit final : public rhi::RenderPass<Blit>, public Technique {
  friend class BasePass;

public:
  Blit(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  // Mixes a and b (by alpha of b).
  [[nodiscard]] FrameGraphResource mix(FrameGraph &, FrameGraphResource a,
                                       FrameGraphResource b);

  // Adds a given source color to the target texture (additive blending).
  [[nodiscard]] FrameGraphResource
  addColor(FrameGraph &, FrameGraphResource target, FrameGraphResource source);

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

private:
  const CommonSamplers &m_samplers;
};

} // namespace gfx
