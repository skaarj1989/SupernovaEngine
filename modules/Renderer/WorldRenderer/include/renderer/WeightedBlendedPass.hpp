#pragma once

#include "Technique.hpp"
#include "TransparencyCompositionPass.hpp"
#include "Renderable.hpp"
#include "CodePair.hpp"

namespace gfx {

struct ViewInfo;
struct ForwardPassInfo;
struct LightingPassFeatures;
struct LightingSettings;

class VertexFormat;
class Material;

class WeightedBlendedPass final : public rhi::RenderPass<WeightedBlendedPass>,
                                  public Technique {
  friend class BasePass;

public:
  explicit WeightedBlendedPass(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  void addGeometryPass(FrameGraph &, FrameGraphBlackboard &, const ViewInfo &,
                       const PropertyGroupOffsets &, const LightingSettings &,
                       const bool softShadows);
  void compose(FrameGraph &, FrameGraphBlackboard &);

  [[nodiscard]] static CodePair buildShaderCode(const rhi::RenderDevice &,
                                                const VertexFormat *,
                                                const Material &,
                                                const LightingPassFeatures &,
                                                const bool writeUserData);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const ForwardPassInfo &) const;

private:
  TransparencyCompositionPass m_compositionPass;
};

} // namespace gfx
