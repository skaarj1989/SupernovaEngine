#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"

class DebugDraw;

namespace gfx {

class DebugDrawPass final : public rhi::RenderPass<DebugDrawPass>,
                            public Technique {
  friend class BasePass;

public:
  explicit DebugDrawPass(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  [[nodiscard]] FrameGraphResource
  addGeometryPass(FrameGraph &, const FrameGraphBlackboard &,
                  const FrameGraphResource target, DebugDraw &);

  struct PassInfo {
    rhi::PixelFormat depthFormat;
    rhi::PixelFormat colorFormat;
    rhi::PrimitiveTopology primitiveTopology;
  };

private:
  [[nodiscard]] rhi::GraphicsPipeline _createPipeline(const PassInfo &) const;
};

}; // namespace gfx
