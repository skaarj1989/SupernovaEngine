#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "glm/ext/vector_float2.hpp"

namespace gfx {

class Blur final : public rhi::RenderPass<Blur>, public Technique {
  friend class BasePass;

public:
  explicit Blur(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  [[nodiscard]] FrameGraphResource addGaussianBlur(FrameGraph &,
                                                   FrameGraphResource input,
                                                   const int32_t iterations,
                                                   const float scale);
  [[nodiscard]] FrameGraphResource
  addTwoPassGaussianBlur(FrameGraph &, FrameGraphResource input,
                         const float radius);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const rhi::PixelFormat colorFormat) const;

  [[nodiscard]] FrameGraphResource _addPass(FrameGraph &,
                                            const FrameGraphResource input,
                                            const glm::vec2 direction);
};

} // namespace gfx
