#include "rhi/Swapchain.hpp"
#include "VkCheck.hpp"

namespace rhi {

void Swapchain::_createSurface(const os::Window &window) {
#if defined(VK_USE_PLATFORM_XLIB_KHR)
  const auto &nativeData = window.getNativeData();
  VkXlibSurfaceCreateInfoKHR surfaceCreateInfo{
    .sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
    .dpy = nativeData.display,
    .window = nativeData.handle,
  };
  VK_CHECK(vkCreateXlibSurfaceKHR(m_instance, &surfaceCreateInfo, nullptr,
                                  &m_surface));
#endif
}

} // namespace rhi
