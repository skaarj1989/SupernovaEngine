#pragma once

#include "rhi/SPIRV.hpp"
#include "glad/vulkan.h"
#include "glm/ext/vector_uint3.hpp"

#include <array>
#include <unordered_map>
#include <optional>

namespace rhi {

struct ShaderReflection {
  void accumulate(SPIRV &&);

  std::optional<glm::uvec3> localSize; // ComputeShader only.

  struct Descriptor {
    VkDescriptorType type{};
    uint32_t count{1};
    VkShaderStageFlags stageFlags{0};
  };
  // Key = binding
  // layout(binding = index)
  using DescriptorSet = std::unordered_map<uint32_t, Descriptor>;
  std::array<DescriptorSet, 4> descriptorSets;
  std::vector<VkPushConstantRange> pushConstantRanges;
};

} // namespace rhi
