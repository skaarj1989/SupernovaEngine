#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "rhi/Extent2D.hpp"
#include "Technique.hpp"
#include "Renderable.hpp"
#include "CodePair.hpp"

namespace gfx {

struct ViewInfo;
struct BaseGeometryPassInfo;

class VertexFormat;
class Material;

class GBufferPass final : public rhi::RenderPass<GBufferPass>,
                          public Technique {
  friend class BasePass;

public:
  explicit GBufferPass(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  void addGeometryPass(FrameGraph &, FrameGraphBlackboard &,
                       const rhi::Extent2D resolution, const ViewInfo &,
                       const PropertyGroupOffsets &);

  [[nodiscard]] static CodePair buildShaderCode(const rhi::RenderDevice &,
                                                const VertexFormat *,
                                                const Material &);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const BaseGeometryPassInfo &) const;
};

} // namespace gfx
