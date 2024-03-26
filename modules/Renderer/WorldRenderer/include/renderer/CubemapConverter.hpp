#pragma once

#include "rhi/ComputePass.hpp"
#include "rhi/Texture.hpp"
#include "Technique.hpp"

namespace gfx {

class CubemapConverter final : public rhi::ComputePass<CubemapConverter>,
                               public Technique {
  friend class BasePass;

public:
  explicit CubemapConverter(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  [[nodiscard]] rhi::Texture equirectangularToCubemap(rhi::CommandBuffer &,
                                                      const rhi::Texture &);

private:
  [[nodiscard]] rhi::ComputePipeline
  _createPipeline(const rhi::PixelFormat colorFormat) const;
};

} // namespace gfx
