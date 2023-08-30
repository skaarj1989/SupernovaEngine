#include "rhi/Swapchain.hpp"
#include "VkCheck.hpp"

namespace rhi {

void Swapchain::_createSurface(const os::Window &window) {
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  const auto &[hInstance, hWnd, _] = window.getNativeData();
  const VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{
    .sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
    .hinstance = hInstance,
    .hwnd = hWnd,
  };
  VK_CHECK(vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, nullptr,
                                   &m_surface));
#endif
}

} // namespace rhi
