#pragma once

#include "Technique.hpp"
#include "Downsampler.hpp"
#include "Upsampler.hpp"

namespace gfx {

class Bloom final : public Technique {
  friend class BasePass;

public:
  Bloom(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  void resample(FrameGraph &, FrameGraphBlackboard &, const float radius);

private:
  Downsampler m_downsampler;
  Upsampler m_upsampler;
};

} // namespace gfx
