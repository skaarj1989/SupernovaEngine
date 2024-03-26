#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"

#include "Cascade.hpp"
#include "Light.hpp"
#include "Renderable.hpp"
#include "CodePair.hpp"

#include "ShadowSettings.hpp"

namespace gfx {

struct BaseGeometryPassInfo;

class PerspectiveCamera;
class VertexFormat;
class Material;

using LightShadowPair = robin_hood::pair<const Light *, int32_t>;
using ShadowMapIndices =
  robin_hood::unordered_map<LightType, std::vector<LightShadowPair>>;

class ShadowRenderer final : public rhi::RenderPass<ShadowRenderer>,
                             public Technique {
  friend class BasePass;

public:
  explicit ShadowRenderer(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  using Settings = ShadowSettings;

  [[nodiscard]] ShadowMapIndices
  update(FrameGraph &, FrameGraphBlackboard &, const PerspectiveCamera &,
         std::span<const Light *> visibleLights,
         std::span<const Renderable> allRenderables,
         const PropertyGroupOffsets &, const Settings &);

  [[nodiscard]] FrameGraphResource
  visualizeCascades(FrameGraph &, const FrameGraphBlackboard &,
                    FrameGraphResource target) const;

  [[nodiscard]] static CodePair buildShaderCode(const rhi::RenderDevice &,
                                                const VertexFormat *,
                                                const Material &);

private:
  [[nodiscard]] std::vector<Cascade> _buildCascadedShadowMaps(
    FrameGraph &, FrameGraphBlackboard &, const PerspectiveCamera &,
    const Light &, std::span<const Renderable> allRenderables,
    const PropertyGroupOffsets &, const Settings::CascadedShadowMaps &);

  [[nodiscard]] FrameGraphResource _addCascadePass(
    FrameGraph &, const FrameGraphBlackboard &, const uint32_t cascadeIndex,
    std::optional<FrameGraphResource> cascadedShadowMaps,
    const RawCamera &lightView, std::vector<const Renderable *> &&,
    const PropertyGroupOffsets &, const Settings::CascadedShadowMaps &);

  [[nodiscard]] std::vector<glm::mat4> _buildSpotLightShadowMaps(
    FrameGraph &, FrameGraphBlackboard &, std::span<const Light *>,
    std::vector<LightShadowPair> &, std::span<const Renderable> allRenderables,
    const PropertyGroupOffsets &, const Settings::SpotLightShadowMaps &);

  [[nodiscard]] FrameGraphResource _addSpotLightPass(
    FrameGraph &, const FrameGraphBlackboard &, const uint32_t index,
    std::optional<FrameGraphResource> shadowMaps, const RawCamera &lightView,
    std::vector<const Renderable *> &&, const PropertyGroupOffsets &,
    const Settings::SpotLightShadowMaps &);

  void _buildOmniLightShadowMaps(FrameGraph &, FrameGraphBlackboard &,
                                 std::span<const Light *>,
                                 std::vector<LightShadowPair> &,
                                 std::span<const Renderable>,
                                 const PropertyGroupOffsets &,
                                 const Settings::OmniShadowMaps &);

  [[nodiscard]] FrameGraphResource _addOmniLightPass(
    FrameGraph &, FrameGraphBlackboard &, const uint32_t index,
    const rhi::CubeFace, std::optional<FrameGraphResource> shadowMaps,
    const Light &light, std::span<const Renderable *>,
    const PropertyGroupOffsets &, const Settings::OmniShadowMaps &);

  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const BaseGeometryPassInfo &) const;

private:
  rhi::GraphicsPipeline m_debugPipeline;
};

} // namespace gfx
