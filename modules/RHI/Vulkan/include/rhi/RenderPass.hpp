#pragma once

#include "BasePass.hpp"
#include "GraphicsPipeline.hpp"

namespace rhi {

template <class TargetPass>
using RenderPass = BasePass<TargetPass, GraphicsPipeline>;

} // namespace rhi
