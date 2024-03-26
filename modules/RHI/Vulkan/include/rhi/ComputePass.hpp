#pragma once

#include "BasePass.hpp"
#include "ComputePipeline.hpp"

namespace rhi {

template <class TargetPass>
using ComputePass = BasePass<TargetPass, ComputePipeline>;

} // namespace rhi
