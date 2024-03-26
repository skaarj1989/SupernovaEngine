#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "Renderable.hpp"
#include "CodePair.hpp"

namespace gfx {

struct CommonSamplers;

struct ViewInfo;
struct BaseGeometryPassInfo;

class VertexFormat;
class Material;

class DecalPass final : public rhi::RenderPass<DecalPass>, public Technique {
  friend class BasePass;

public:
  DecalPass(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

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
