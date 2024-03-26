#include "rhi/Swapchain.hpp"
#include "os/Window.hpp"
#include "VkCheck.hpp"
#include "tracy/Tracy.hpp"

// https://www.intel.com/content/www/us/en/developer/articles/training/api-without-secrets-introduction-to-vulkan-part-2.html?language=en#inpage-nav-4-5

namespace rhi {

namespace {

struct SurfaceInfo {
  VkSurfaceCapabilitiesKHR capabilities{};
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};
[[nodiscard]] auto getSurfaceInfo(const VkPhysicalDevice physicalDevice,
                                  const VkSurfaceKHR surface) {
  assert(physicalDevice != VK_NULL_HANDLE && surface != VK_NULL_HANDLE);

  SurfaceInfo surfaceInfo;
  VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    physicalDevice, surface, &surfaceInfo.capabilities));

  uint32_t numFormats{0};
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface,
                                                &numFormats, nullptr));
  surfaceInfo.formats.resize(numFormats);
  VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(
    physicalDevice, surface, &numFormats, surfaceInfo.formats.data()));

  uint32_t numPresentModes{0};
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
    physicalDevice, surface, &numPresentModes, nullptr));
  surfaceInfo.presentModes.resize(numPresentModes);
  VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(
    physicalDevice, surface, &numPresentModes,
    surfaceInfo.presentModes.data()));

  return surfaceInfo;
}

[[nodiscard]] Extent2D fromVk(const VkExtent2D extent) {
  return {extent.width, extent.height};
}

[[nodiscard]] auto getPresentMode(const VerticalSync vsync) {
  switch (vsync) {
    using enum VerticalSync;

  case Disabled:
    return VK_PRESENT_MODE_IMMEDIATE_KHR;
  case Enabled:
    return VK_PRESENT_MODE_FIFO_KHR;
  case Adaptive:
    return VK_PRESENT_MODE_MAILBOX_KHR;
  }
  assert(false);
  return VK_PRESENT_MODE_FIFO_KHR;
}

} // namespace

//
// Swapchain class:
//

Swapchain::Swapchain(Swapchain &&other) noexcept
    : m_instance{other.m_instance}, m_physicalDevice{other.m_physicalDevice},
      m_device{other.m_device}, m_surface{other.m_surface},
      m_handle{other.m_handle}, m_format{other.m_format},
      m_verticalSync{other.m_verticalSync},
      m_buffers{std::move(other.m_buffers)},
      m_currentImageIndex{other.m_currentImageIndex} {
  other.m_instance = VK_NULL_HANDLE;
  other.m_physicalDevice = VK_NULL_HANDLE;
  other.m_device = VK_NULL_HANDLE;

  other.m_surface = VK_NULL_HANDLE;
  other.m_handle = VK_NULL_HANDLE;

  other.m_currentImageIndex = 0;
}
Swapchain::~Swapchain() { _destroy(); }

Swapchain &Swapchain::operator=(Swapchain &&rhs) noexcept {
  if (this != &rhs) {
    _destroy();

    std::swap(m_instance, rhs.m_instance);
    std::swap(m_physicalDevice, rhs.m_physicalDevice);
    std::swap(m_device, rhs.m_device);

    std::swap(m_surface, rhs.m_surface);
    std::swap(m_handle, rhs.m_handle);

    std::swap(m_format, rhs.m_format);
    std::swap(m_verticalSync, rhs.m_verticalSync);

    std::swap(m_buffers, rhs.m_buffers);
    std::swap(m_currentImageIndex, rhs.m_currentImageIndex);
  }
  return *this;
}

Swapchain::operator bool() const { return m_handle != VK_NULL_HANDLE; }

PixelFormat Swapchain::getPixelFormat() const {
  return m_handle ? m_buffers.back().getPixelFormat() : PixelFormat::Undefined;
}
Extent2D Swapchain::getExtent() const {
  return m_handle ? m_buffers.back().getExtent() : Extent2D{};
}

std::size_t Swapchain::getNumBuffers() const { return m_buffers.size(); }

const std::vector<Texture> &Swapchain::getBuffers() const { return m_buffers; }
const Texture &Swapchain::getBuffer(const uint32_t i) const {
  return m_buffers[i];
}

uint32_t Swapchain::getCurrentBufferIndex() const {
  return m_currentImageIndex;
}
Texture &Swapchain::getCurrentBuffer() {
  return m_buffers[m_currentImageIndex];
}

void Swapchain::recreate(const std::optional<VerticalSync> vsync) {
  m_buffers.clear();
  _create(m_format, vsync.value_or(m_verticalSync));
}

bool Swapchain::acquireNextImage(const VkSemaphore imageAcquired) {
  assert(m_handle != VK_NULL_HANDLE);
  ZoneScopedN("RHI::AcquireNextImage");

  const auto result =
    vkAcquireNextImageKHR(m_device, m_handle, UINT64_MAX, imageAcquired,
                          VK_NULL_HANDLE, &m_currentImageIndex);
  switch (result) {
  case VK_ERROR_OUT_OF_DATE_KHR:
    recreate();
    [[fallthrough]];
  case VK_SUBOPTIMAL_KHR:
  case VK_SUCCESS:
    return true;
  }

  assert(false);
  return false;
}

//
// (private):
//

Swapchain::Swapchain(const VkInstance instance,
                     const VkPhysicalDevice physicalDevice,
                     const VkDevice device, const os::Window &window,
                     const Format format, const VerticalSync vsync)
    : m_instance{instance}, m_physicalDevice{physicalDevice}, m_device{device} {
  _createSurface(window);
  _create(format, vsync);
}

void Swapchain::_create(const Format format, const VerticalSync vsync) {
  const auto oldSwapchain = std::exchange(m_handle, VK_NULL_HANDLE);

  const auto surfaceInfo = getSurfaceInfo(m_physicalDevice, m_surface);
  if (const auto extent = fromVk(surfaceInfo.capabilities.currentExtent);
      extent) {
    const VkSurfaceFormatKHR kSurfaceDefaultFormat{
      .format = format == Format::Linear ? VK_FORMAT_B8G8R8A8_UNORM
                                         : VK_FORMAT_B8G8R8A8_SRGB,
      .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    };
    const VkSwapchainCreateInfoKHR createInfo{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = m_surface,
      .minImageCount = std::clamp(3u, surfaceInfo.capabilities.minImageCount,
                                  surfaceInfo.capabilities.maxImageCount > 0
                                    ? surfaceInfo.capabilities.maxImageCount
                                    : 8u),
      .imageFormat = kSurfaceDefaultFormat.format,
      .imageColorSpace = kSurfaceDefaultFormat.colorSpace,
      .imageExtent = static_cast<VkExtent2D>(extent),
      .imageArrayLayers = 1, // No stereo rendering.
      .imageUsage =
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = getPresentMode(vsync),
      .clipped = VK_TRUE,
      .oldSwapchain = oldSwapchain,
    };
    VK_CHECK(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_handle));
    _buildBuffers(extent, PixelFormat(createInfo.imageFormat));
    m_format = format;
    m_verticalSync = vsync;
  }

  if (oldSwapchain != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(m_device, oldSwapchain, nullptr);
  }
}
void Swapchain::_buildBuffers(const Extent2D extent,
                              const PixelFormat pixelFormat) {
  assert(m_buffers.empty());

  uint32_t imageCount{0};
  VK_CHECK(vkGetSwapchainImagesKHR(m_device, m_handle, &imageCount, nullptr));
  assert(imageCount > 0);
  std::vector<VkImage> images(imageCount);
  VK_CHECK(
    vkGetSwapchainImagesKHR(m_device, m_handle, &imageCount, images.data()));

  m_buffers.reserve(images.size());
  for (auto image : images) {
    m_buffers.emplace_back(Texture{m_device, image, extent, pixelFormat});
  }
}
void Swapchain::_destroy() {
  m_buffers.clear();

  if (m_handle != VK_NULL_HANDLE) {
    vkDestroySwapchainKHR(m_device, m_handle, nullptr);
    m_handle = VK_NULL_HANDLE;
  }
  if (m_surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    m_surface = VK_NULL_HANDLE;
  }

  m_instance = VK_NULL_HANDLE;
  m_physicalDevice = VK_NULL_HANDLE;
  m_device = VK_NULL_HANDLE;

  m_currentImageIndex = 0;
}

//
// Utility:
//

Rect2D getRenderArea(const Swapchain &swapchain) {
  return Rect2D{.offset = {0, 0}, .extent = {swapchain.getExtent()}};
}

} // namespace rhi
