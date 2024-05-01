#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "Renderable.hpp"
#include "CodePair.hpp"

namespace gfx {

struct ViewInfo;
struct ForwardPassInfo;
struct LightingPassFeatures;
struct LightingSettings;

class VertexFormat;
class Material;

class TransparencyPass final : public rhi::RenderPass<TransparencyPass>,
                               public Technique {
  friend class BasePass;

public:
  explicit TransparencyPass(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  [[nodiscard]] std::optional<FrameGraphResource>
  addGeometryPass(FrameGraph &, FrameGraphBlackboard &,
                  const FrameGraphResource target, const ViewInfo &,
                  const PropertyGroupOffsets &, const LightingSettings &,
                  bool const softShadows);

  [[nodiscard]] static CodePair buildShaderCode(const rhi::RenderDevice &,
                                                const VertexFormat *,
                                                const Material &,
                                                const LightingPassFeatures &,
                                                const bool writeUserData);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const ForwardPassInfo &) const;
};

} // namespace gfx
