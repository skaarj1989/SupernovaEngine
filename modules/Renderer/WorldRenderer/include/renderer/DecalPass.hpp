#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "BaseGeometryPassInfo.hpp"
#include "ViewInfo.hpp"
#include "CodePair.hpp"

namespace gfx {

struct CommonSamplers;

class DecalPass final : public rhi::RenderPass<DecalPass>, public Technique {
  friend class BasePass;

public:
  DecalPass(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  void addGeometryPass(FrameGraph &, FrameGraphBlackboard &, const ViewInfo &,
                       const PropertyGroupOffsets &);

  [[nodiscard]] static CodePair buildShaderCode(const rhi::RenderDevice &,
                                                const VertexFormat *,
                                                const Material &);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const BaseGeometryPassInfo &) const;

private:
  const CommonSamplers &m_samplers;
};

} // namespace gfx
