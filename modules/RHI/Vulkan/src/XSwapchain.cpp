#include "rhi/Swapchain.hpp"
#include "VkCheck.hpp"

namespace rhi {

void Swapchain::_createSurface(const os::Window &window) {
#if defined(VK_USE_PLATFORM_XCB_KHR)
  const auto [connection, id] = window.getNativeData();
  VkXcbSurfaceCreateInfoKHR surfaceCreateInfo{
    .sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
    .connection = connection,
    .window = id,
  };
  VK_CHECK(
    vkCreateXcbSurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface));
#endif
}

} // namespace rhi
