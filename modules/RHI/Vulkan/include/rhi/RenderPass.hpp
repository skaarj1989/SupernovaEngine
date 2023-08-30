#pragma once

#include "BasePass.hpp"

namespace rhi {

template <class TargetPass>
using RenderPass = BasePass<TargetPass, GraphicsPipeline>;

} // namespace rhi
