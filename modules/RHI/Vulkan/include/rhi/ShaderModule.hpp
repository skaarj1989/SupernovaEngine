#pragma once

#include "SPIRV.hpp"
#include "glad/vulkan.h"

namespace rhi {

class RenderDevice;

class ShaderModule final {
  friend class RenderDevice; // Calls private constructors.

public:
  ShaderModule(const ShaderModule &) = delete;
  ShaderModule(ShaderModule &&) noexcept;
  ~ShaderModule();

  ShaderModule &operator=(const ShaderModule &) = delete;
  ShaderModule &operator=(ShaderModule &&) noexcept;

  [[nodiscard]] explicit operator bool() const;
  [[nodiscard]] explicit operator VkShaderModule() const;

private:
  ShaderModule() = default;
  ShaderModule(const VkDevice, const SPIRV &);

  void _destroy() noexcept;

private:
  VkDevice m_device{VK_NULL_HANDLE};
  VkShaderModule m_handle{VK_NULL_HANDLE};
};

} // namespace rhi
