#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "BaseGeometryPassInfo.hpp"
#include "ViewInfo.hpp"
#include "CodePair.hpp"

namespace gfx {

class Batch;

class GBufferPass final : public rhi::RenderPass<GBufferPass>,
                          public Technique {
  friend class BasePass;

public:
  explicit GBufferPass(rhi::RenderDevice &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  void addGeometryPass(FrameGraph &, FrameGraphBlackboard &,
                       rhi::Extent2D resolution, const ViewInfo &,
                       const PropertyGroupOffsets &);

  [[nodiscard]] static CodePair buildShaderCode(const rhi::RenderDevice &,
                                                const VertexFormat *,
                                                const Material &);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const BaseGeometryPassInfo &) const;
};

} // namespace gfx
