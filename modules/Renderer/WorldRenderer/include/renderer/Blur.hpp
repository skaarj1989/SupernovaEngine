#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "glm/ext/vector_float2.hpp"

namespace gfx {

struct CommonSamplers;

class Blur final : public rhi::RenderPass<Blur>, public Technique {
  friend class BasePass;

public:
  Blur(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  [[nodiscard]] FrameGraphResource addGaussianBlur(FrameGraph &,
                                                   FrameGraphResource input,
                                                   int32_t iterations,
                                                   float scale);
  [[nodiscard]] FrameGraphResource
  addTwoPassGaussianBlur(FrameGraph &, FrameGraphResource input, float radius);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(rhi::PixelFormat colorFormat) const;

  [[nodiscard]] FrameGraphResource
  _addPass(FrameGraph &, FrameGraphResource input, glm::vec2 direction);

private:
  const CommonSamplers &m_samplers;
};

} // namespace gfx
