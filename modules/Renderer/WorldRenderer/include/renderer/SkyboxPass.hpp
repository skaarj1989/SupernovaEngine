#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"

namespace gfx {

class SkyboxPass final : public rhi::RenderPass<SkyboxPass>, public Technique {
  friend class BasePass;

public:
  explicit SkyboxPass(rhi::RenderDevice &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  [[nodiscard]] FrameGraphResource addPass(FrameGraph &,
                                           const FrameGraphBlackboard &,
                                           FrameGraphResource target);

  struct PassInfo {
    rhi::PixelFormat depthFormat;
    rhi::PixelFormat colorFormat;
    rhi::TextureType textureType;
  };

private:
  rhi::GraphicsPipeline _createPipeline(const PassInfo &) const;
};

} // namespace gfx
