#include "rhi/ShaderModule.hpp"
#include "VkCheck.hpp"

namespace rhi {

ShaderModule::ShaderModule(ShaderModule &&other) noexcept
    : m_device{other.m_device}, m_handle{other.m_handle} {
  other.m_device = VK_NULL_HANDLE;
  other.m_handle = VK_NULL_HANDLE;
}
ShaderModule::~ShaderModule() { _destroy(); }

ShaderModule &ShaderModule::operator=(ShaderModule &&rhs) noexcept {
  if (this != &rhs) {
    _destroy();

    std::swap(m_device, rhs.m_device);
    std::swap(m_handle, rhs.m_handle);
  }
  return *this;
}

ShaderModule::operator bool() const { return m_handle != VK_NULL_HANDLE; }
ShaderModule::operator VkShaderModule() const { return m_handle; }

ShaderModule::ShaderModule(VkDevice device, const SPIRV &spv)
    : m_device{device} {
  assert(device != VK_NULL_HANDLE && !spv.empty());

  const VkShaderModuleCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .codeSize = sizeof(uint32_t) * spv.size(),
    .pCode = spv.data(),
  };
  VK_CHECK(vkCreateShaderModule(m_device, &createInfo, nullptr, &m_handle));
}

void ShaderModule::_destroy() noexcept {
  if (m_handle != VK_NULL_HANDLE) {
    vkDestroyShaderModule(m_device, m_handle, nullptr);

    m_device = VK_NULL_HANDLE;
    m_handle = VK_NULL_HANDLE;
  }
}

} // namespace rhi
