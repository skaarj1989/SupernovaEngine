#pragma once

#include "ScopedEnumFlags.hpp"
#include "glad/vulkan.h"

namespace rhi {

enum class ShaderType {
  Vertex,
  Geometry,
  Fragment,
  Compute,
};

[[nodiscard]] const char *toString(const ShaderType);

enum class ShaderStages : VkShaderStageFlags {
  Vertex = VK_SHADER_STAGE_VERTEX_BIT,
  Geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
  Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
  Compute = VK_SHADER_STAGE_COMPUTE_BIT
};

[[nodiscard]] ShaderStages getStage(const ShaderType);
[[nodiscard]] uint8_t countStages(const ShaderStages);

} // namespace rhi

template <> struct has_flags<rhi::ShaderStages> : std::true_type {};
