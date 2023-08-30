#pragma once

#include "BasePass.hpp"

namespace rhi {

template <class TargetPass>
using ComputePass = BasePass<TargetPass, ComputePipeline>;

} // namespace rhi
