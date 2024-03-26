#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "Renderable.hpp"
#include "CodePair.hpp"

#include "FrameGraphData/ReflectiveShadowMap.hpp"
#include "FrameGraphData/LightPropagationVolumes.hpp"

#include "glm/ext/vector_uint3.hpp"
#include "glm/ext/vector_float3.hpp"

namespace gfx {

struct CommonSamplers;

struct BaseGeometryPassInfo;

class PerspectiveCamera;
struct RawCamera;
struct Light;
struct Grid;

class VertexFormat;
class Material;

class GlobalIllumination final : public rhi::RenderPass<GlobalIllumination>,
                                 public Technique {
  friend class BasePass;

public:
  GlobalIllumination(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  void update(FrameGraph &, FrameGraphBlackboard &, const Grid &,
              const PerspectiveCamera &, const Light &,
              std::span<const Renderable>, const PropertyGroupOffsets &,
              uint32_t numPropagations);

  [[nodiscard]] FrameGraphResource
  addDebugPass(FrameGraph &, FrameGraphBlackboard &, FrameGraphResource target);

  [[nodiscard]] static CodePair buildShaderCode(const rhi::RenderDevice &,
                                                const VertexFormat *,
                                                const Material &);

private:
  [[nodiscard]] ReflectiveShadowMapData _addReflectiveShadowMapPass(
    FrameGraph &, FrameGraphBlackboard &, const RawCamera &lightView,
    const glm::vec3 lightIntensity, std::vector<const Renderable *> &&,
    const PropertyGroupOffsets &);

  [[nodiscard]] LightPropagationVolumesData _addRadianceInjectionPass(
    FrameGraph &, const FrameGraphResource sceneGridBlock,
    const ReflectiveShadowMapData &, const glm::uvec3 gridSize);
  LightPropagationVolumesData _addRadiancePropagationPass(
    FrameGraph &, const FrameGraphResource sceneGridBlock,
    const LightPropagationVolumesData &, const glm::uvec3 gridSize,
    const uint32_t iteration);

  rhi::GraphicsPipeline _createPipeline(const BaseGeometryPassInfo &) const;

private:
  const CommonSamplers &m_samplers;

  rhi::GraphicsPipeline m_radianceInjectionPipeline;
  rhi::GraphicsPipeline m_radiancePropagationPipeline;

  rhi::GraphicsPipeline m_debugPipeline;
};

} // namespace gfx
