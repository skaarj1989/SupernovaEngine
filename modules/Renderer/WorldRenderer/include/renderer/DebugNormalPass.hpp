#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"

namespace gfx {

struct ViewInfo;
struct BaseGeometryPassInfo;

class DebugNormalPass final : public rhi::RenderPass<DebugNormalPass>,
                              public Technique {
  friend class BasePass;

public:
  explicit DebugNormalPass(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  [[nodiscard]] FrameGraphResource addGeometryPass(FrameGraph &,
                                                   const FrameGraphBlackboard &,
                                                   FrameGraphResource target,
                                                   const ViewInfo &);

private:
  [[nodiscard]] rhi::GraphicsPipeline
  _createPipeline(const BaseGeometryPassInfo &) const;
};

}; // namespace gfx
