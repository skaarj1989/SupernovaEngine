#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "ForwardPassInfo.hpp"
#include "ViewInfo.hpp"
#include "LightingSettings.hpp"
#include "CodePair.hpp"

namespace gfx {

struct CommonSamplers;

class TransparencyPass final : public rhi::RenderPass<TransparencyPass>,
                               public Technique {
  friend class BasePass;

public:
  TransparencyPass(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  [[nodiscard]] std::optional<FrameGraphResource>
  addGeometryPass(FrameGraph &, const FrameGraphBlackboard &,
                  FrameGraphResource target, const ViewInfo &,
                  const PropertyGroupOffsets &, const LightingSettings &,
                  bool softShadows);

  [[nodiscard]] static CodePair buildShaderCode(const rhi::RenderDevice &,
                                                const VertexFormat *,
                                                const Material &,
                                                const LightingPassFeatures &);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const ForwardPassInfo &) const;

private:
  const CommonSamplers &m_samplers;
};

} // namespace gfx
