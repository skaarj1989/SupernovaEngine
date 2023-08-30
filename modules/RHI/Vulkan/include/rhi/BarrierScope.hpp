#pragma once

#include "rhi/PipelineStage.hpp"
#include "rhi/Access.hpp"

namespace rhi {

struct BarrierScope {
  PipelineStages stageMask{PipelineStages::None};
  Access accessMask{Access::None};

  bool operator==(const BarrierScope &) const = default;
};

constexpr auto kInitialBarrierScope = BarrierScope{
  .stageMask = PipelineStages::Top,
  .accessMask = Access::None,
};
constexpr auto kFatScope = rhi::BarrierScope{
  .stageMask = PipelineStages::AllCommands,
  .accessMask = Access::MemoryRead | Access::MemoryWrite,
};

} // namespace rhi
