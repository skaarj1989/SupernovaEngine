#pragma once

#include "glad/vulkan.h"

namespace rhi {

// https://registry.khronos.org/vulkan/specs/1.3/html/chap20.html#VkPrimitiveTopology
enum class PrimitiveTopology {
  PointList = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
  LineList = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
  TriangleList = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
  TriangleStrip = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
  TriangleFan = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
};

[[nodiscard]] const char *toString(PrimitiveTopology);

} // namespace rhi
