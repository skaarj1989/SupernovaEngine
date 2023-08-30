#pragma once

#include "rhi/ComputePass.hpp"
#include "Technique.hpp"

namespace gfx {

class CubemapConverter final : public rhi::ComputePass<CubemapConverter>,
                               public Technique {
  friend class BasePass;

public:
  explicit CubemapConverter(rhi::RenderDevice &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  [[nodiscard]] rhi::Texture equirectangularToCubemap(rhi::CommandBuffer &,
                                                      const rhi::Texture &);

private:
  [[nodiscard]] rhi::ComputePipeline
  _createPipeline(rhi::PixelFormat colorFormat) const;
};

} // namespace gfx
