#include "ShaderReflection.hpp"
#include "spirv_glsl.hpp"

// https://github.com/KhronosGroup/SPIRV-Cross/wiki/Reflection-API-user-guide

namespace rhi {

namespace {

[[nodiscard]] VkShaderStageFlags
toVk(const spv::ExecutionModel executionModel) {
  switch (executionModel) {
  case spv::ExecutionModelVertex:
    return VK_SHADER_STAGE_VERTEX_BIT;
  case spv::ExecutionModelGeometry:
    return VK_SHADER_STAGE_GEOMETRY_BIT;
  case spv::ExecutionModelFragment:
    return VK_SHADER_STAGE_FRAGMENT_BIT;
  case spv::ExecutionModelGLCompute:
    return VK_SHADER_STAGE_COMPUTE_BIT;
  }
  assert(false);
  return spv::ExecutionModelMax;
}

[[nodiscard]] auto getLocalSize(const spirv_cross::CompilerGLSL &compiler) {
  glm::uvec3 localSize{};
  for (auto i = 0; i < decltype(localSize)::length(); ++i) {
    localSize[i] =
      compiler.get_execution_mode_argument(spv::ExecutionModeLocalSize, i);
  }
  return localSize;
}

} // namespace

void ShaderReflection::accumulate(SPIRV &&spv) {
  spirv_cross::CompilerGLSL compiler{std::move(spv)};

  const auto stageFlag = toVk(compiler.get_execution_model());
  if (stageFlag & VK_SHADER_STAGE_COMPUTE_BIT) {
    localSize = getLocalSize(compiler);
  }
  const auto shaderResources = compiler.get_shader_resources();

  const auto addResource = [&](const spirv_cross::Resource &r,
                               VkDescriptorType descriptorType) {
    const auto &type = compiler.get_type(r.type_id);
    const auto set =
      compiler.get_decoration(r.id, spv::DecorationDescriptorSet);
    const auto binding = compiler.get_decoration(r.id, spv::DecorationBinding);

    auto [it, emplaced] =
      descriptorSets[set].try_emplace(binding, descriptorType);
    auto &resource = it->second;
    if (emplaced) resource.count = type.array.empty() ? 1 : type.array[0];
    resource.stageFlags |= stageFlag;
  };

#define ADD_RESOURCES(Member, EnumValue)                                       \
  for (const auto &r : shaderResources.Member) {                               \
    addResource(r, VK_DESCRIPTOR_TYPE_##EnumValue);                            \
  }

  ADD_RESOURCES(separate_samplers, SAMPLER)
  ADD_RESOURCES(sampled_images, COMBINED_IMAGE_SAMPLER)
  ADD_RESOURCES(separate_images, SAMPLED_IMAGE)
  ADD_RESOURCES(storage_images, STORAGE_IMAGE)
  ADD_RESOURCES(uniform_buffers, UNIFORM_BUFFER)
  ADD_RESOURCES(storage_buffers, STORAGE_BUFFER)

#undef ADD_RESOURCES

  if (!shaderResources.push_constant_buffers.empty()) {
    const auto &pc = shaderResources.push_constant_buffers.front();
    const auto &type = compiler.get_type(pc.base_type_id);

    VkPushConstantRange range{.stageFlags = stageFlag, .offset = 0, .size = 0};
    for (auto i = 0u; i < type.member_types.size(); ++i) {
      if (i == 0) range.offset = compiler.type_struct_member_offset(type, i);
      range.size += compiler.get_declared_struct_member_size(type, i);
    }
    pushConstantRanges.emplace_back(range);
  }
}

} // namespace rhi
