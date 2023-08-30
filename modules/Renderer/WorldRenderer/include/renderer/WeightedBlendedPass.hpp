#pragma once

#include "Technique.hpp"
#include "ForwardPassInfo.hpp"
#include "ViewInfo.hpp"
#include "LightingSettings.hpp"
#include "CodePair.hpp"
#include "TransparencyCompositionPass.hpp"

namespace gfx {

struct CommonSamplers;

class WeightedBlendedPass final : public rhi::RenderPass<WeightedBlendedPass>,
                                  public Technique {
  friend class BasePass;

public:
  WeightedBlendedPass(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  void addGeometryPass(FrameGraph &, FrameGraphBlackboard &, const ViewInfo &,
                       const PropertyGroupOffsets &, const LightingSettings &,
                       bool softShadows);
  void compose(FrameGraph &, FrameGraphBlackboard &);

  [[nodiscard]] static CodePair buildShaderCode(const rhi::RenderDevice &,
                                                const VertexFormat *,
                                                const Material &,
                                                const LightingPassFeatures &);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const ForwardPassInfo &) const;

private:
  const CommonSamplers &m_samplers;
  TransparencyCompositionPass m_compositionPass;
};

} // namespace gfx
