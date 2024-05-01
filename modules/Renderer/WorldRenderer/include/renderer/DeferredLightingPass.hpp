#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"

namespace gfx {

struct LightingSettings;
struct LightingPassFeatures;

class DeferredLightingPass final : public rhi::RenderPass<DeferredLightingPass>,
                                   public Technique {
  friend class BasePass;

public:
  explicit DeferredLightingPass(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  [[nodiscard]] FrameGraphResource
  addPass(FrameGraph &, const FrameGraphBlackboard &, const LightingSettings &,
          const bool softShadows, const bool irradianceOnly);

private:
  rhi::GraphicsPipeline _createPipeline(const rhi::PixelFormat colorFormat,
                                        const LightingPassFeatures &) const;
};

} // namespace gfx
