#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "Renderable.hpp"
#include "CodePair.hpp"

namespace gfx {

struct CommonSamplers;

struct ViewInfo;
struct ForwardPassInfo;
struct LightingPassFeatures;
struct LightingSettings;

class VertexFormat;
class Material;

class TransmissionPass final : public rhi::RenderPass<TransmissionPass>,
                               public Technique {
  friend class BasePass;

public:
  TransmissionPass(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  [[nodiscard]] std::optional<FrameGraphResource>
  addGeometryPass(FrameGraph &, FrameGraphBlackboard &,
                  const FrameGraphResource sceneColor, const ViewInfo &,
                  const PropertyGroupOffsets &, const LightingSettings &,
                  const bool softShadows);

  [[nodiscard]] static CodePair buildShaderCode(const rhi::RenderDevice &,
                                                const VertexFormat *,
                                                const Material &,
                                                const LightingPassFeatures &,
                                                const bool writeUserData);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const ForwardPassInfo &) const;

private:
  const CommonSamplers &m_samplers;
};

} // namespace gfx
