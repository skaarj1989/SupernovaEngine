#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "DebugDraw.hpp"

namespace gfx {

class DebugDrawPass final : public rhi::RenderPass<DebugDrawPass>,
                            public Technique {
  friend class BasePass;

public:
  explicit DebugDrawPass(rhi::RenderDevice &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  [[nodiscard]] FrameGraphResource addGeometryPass(FrameGraph &,
                                                   const FrameGraphBlackboard &,
                                                   FrameGraphResource target,
                                                   DebugDraw &);

  struct PassInfo {
    rhi::PixelFormat depthFormat;
    rhi::PixelFormat colorFormat;
    rhi::PrimitiveTopology primitiveTopology;
  };

private:
  [[nodiscard]] rhi::GraphicsPipeline _createPipeline(const PassInfo &) const;
};

}; // namespace gfx
