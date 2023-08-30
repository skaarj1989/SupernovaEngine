#pragma once

#include "glad/vulkan.h"

namespace rhi {

// https://registry.khronos.org/vulkan/specs/1.3/html/chap13.html#VkCompareOp
enum class CompareOp {
  Never = VK_COMPARE_OP_NEVER,
  Less = VK_COMPARE_OP_LESS,
  Equal = VK_COMPARE_OP_EQUAL,
  LessOrEqual = VK_COMPARE_OP_LESS_OR_EQUAL,
  Greater = VK_COMPARE_OP_GREATER,
  NotEqual = VK_COMPARE_OP_NOT_EQUAL,
  GreaterOrEqual = VK_COMPARE_OP_GREATER_OR_EQUAL,
  Always = VK_COMPARE_OP_ALWAYS
};

} // namespace rhi
