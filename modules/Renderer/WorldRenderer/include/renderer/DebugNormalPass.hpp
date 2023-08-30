#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "BaseGeometryPassInfo.hpp"
#include "ViewInfo.hpp"

namespace gfx {

class DebugNormalPass final : public rhi::RenderPass<DebugNormalPass>,
                              public Technique {
  friend class BasePass;

public:
  explicit DebugNormalPass(rhi::RenderDevice &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  [[nodiscard]] FrameGraphResource addGeometryPass(FrameGraph &,
                                                   const FrameGraphBlackboard &,
                                                   FrameGraphResource target,
                                                   const ViewInfo &);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const BaseGeometryPassInfo &) const;
};

}; // namespace gfx
