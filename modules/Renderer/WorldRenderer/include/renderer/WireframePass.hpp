#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"

namespace gfx {

struct ViewInfo;
struct BaseGeometryPassInfo;

class WireframePass final : public rhi::RenderPass<WireframePass>,
                            public Technique {
  friend class BasePass;

public:
  explicit WireframePass(rhi::RenderDevice &);

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
