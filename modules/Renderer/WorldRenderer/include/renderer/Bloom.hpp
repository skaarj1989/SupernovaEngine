#pragma once

#include "Technique.hpp"
#include "Downsampler.hpp"
#include "Upsampler.hpp"

namespace gfx {

class Bloom final : public Technique {
  friend class BasePass;

public:
  Bloom(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  void resample(FrameGraph &, FrameGraphBlackboard &, float radius);

private:
  Downsampler m_downsampler;
  Upsampler m_upsampler;
};

} // namespace gfx
