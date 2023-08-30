#pragma once

#include "fg/Fwd.hpp"
#include "rhi/ComputePass.hpp"
#include "Technique.hpp"

namespace gfx {

class Blit;

class TiledLighting : public Technique {
public:
  explicit TiledLighting(rhi::RenderDevice &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  using TileSize = uint32_t;

  void cullLights(FrameGraph &, FrameGraphBlackboard &, TileSize);

  [[nodiscard]] FrameGraphResource addDebugOverlay(FrameGraph &,
                                                   FrameGraphBlackboard &,
                                                   Blit &,
                                                   FrameGraphResource target);

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
    buildFrustums(FrameGraph &, FrameGraphBlackboard &, const PassInfo &);

  private:
    [[nodiscard]] rhi::ComputePipeline _createPipeline(TileSize) const;
  };

  class LightCuller final : public rhi::ComputePass<LightCuller> {
    friend class BasePass;

  public:
    explicit LightCuller(rhi::RenderDevice &);

    void cullLights(FrameGraph &, FrameGraphBlackboard &,
                    FrameGraphResource gridFrustums, const PassInfo &);

  private:
    [[nodiscard]] rhi::ComputePipeline _createPipeline(TileSize) const;
  };

private:
  FrustumBuilder m_frustumBuilder;
  LightCuller m_lightCuller;
};

} // namespace gfx
