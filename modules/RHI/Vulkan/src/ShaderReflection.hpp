#pragma once

#include "rhi/ShaderType.hpp"
#include "rhi/SPIRV.hpp"
#include "rhi/ResourceIndices.hpp"
#include "glad/vulkan.h"
#include "glm/ext/vector_uint3.hpp"

#include <array>
#include <unordered_map>
#include <optional>
#include <string>

namespace rhi {

struct ShaderReflection {
  void accumulate(const SPIRV &,
                  const std::pair<ShaderType, std::string> &entryPoint);

  std::optional<glm::uvec3> localSize; // ComputeShader only.

  struct Descriptor {
    VkDescriptorType type{};
    uint32_t count{1};
    VkShaderStageFlags stageFlags{0};
  };
  // Key = binding
  // layout(binding = index)
  using DescriptorSet = std::unordered_map<BindingIndex, Descriptor>;
  std::array<DescriptorSet, 4> descriptorSets;
  std::vector<VkPushConstantRange> pushConstantRanges;
};

} // namespace rhi
