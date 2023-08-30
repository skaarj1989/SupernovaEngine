#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "LightingSettings.hpp"
#include "LightingPassFeatures.hpp"

namespace gfx {

struct CommonSamplers;

class DeferredLightingPass final : public rhi::RenderPass<DeferredLightingPass>,
                                   public Technique {
  friend class BasePass;

public:
  DeferredLightingPass(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  [[nodiscard]] FrameGraphResource
  addPass(FrameGraph &, const FrameGraphBlackboard &, const LightingSettings &,
          bool softShadows, bool irradianceOnly);

  struct PassInfo {
    rhi::PixelFormat colorFormat;
    LightingPassFeatures features;
  };

private:
  rhi::GraphicsPipeline _createPipeline(const PassInfo &) const;

private:
  const CommonSamplers &m_samplers;
};

} // namespace gfx
