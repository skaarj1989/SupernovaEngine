#pragma once

#include "fg/Fwd.hpp"
#include "rhi/ComputePass.hpp"
#include "Technique.hpp"

namespace gfx {

class Blit;

class TiledLighting : public Technique {
public:
  explicit TiledLighting(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  using TileSize = uint32_t;

  void cullLights(FrameGraph &, FrameGraphBlackboard &, const TileSize);

  [[nodiscard]] FrameGraphResource
  addDebugOverlay(FrameGraph &, const FrameGraphBlackboard &, Blit &,
                  const FrameGraphResource target);

private:
  struct PassInfo {
    TileSize tileSize;
    uint32_t numFrustums;
    glm::uvec2 gridSize;
  };

  class FrustumBuilder final : public rhi::ComputePass<FrustumBuilder> {
    friend class BasePass;

  public:
    explicit FrustumBuilder(rhi::RenderDevice &);

    [[nodiscard]] FrameGraphResource
    buildFrustums(FrameGraph &, const FrameGraphBlackboard &, const PassInfo &);

  private:
    [[nodiscard]] rhi::ComputePipeline _createPipeline(const TileSize) const;
  };

  class LightCuller final : public rhi::ComputePass<LightCuller> {
    friend class BasePass;

  public:
    explicit LightCuller(rhi::RenderDevice &);

    void cullLights(FrameGraph &, FrameGraphBlackboard &,
                    const FrameGraphResource gridFrustums, const PassInfo &);

  private:
    [[nodiscard]] rhi::ComputePipeline _createPipeline(const TileSize) const;
  };

private:
  FrustumBuilder m_frustumBuilder;
  LightCuller m_lightCuller;
};

} // namespace gfx
