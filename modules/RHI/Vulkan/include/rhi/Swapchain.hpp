#pragma once

#include "Rect2D.hpp"
#include "Texture.hpp"

namespace os {
class Window;
}

namespace rhi {

enum class VerticalSync { Disabled, Enabled, Adaptive };

class Swapchain final {
  friend class RenderDevice; // Calls the private constructor.

public:
  Swapchain() = default;
  Swapchain(const Swapchain &) = delete;
  Swapchain(Swapchain &&) noexcept;
  ~Swapchain();

  Swapchain &operator=(const Swapchain &) = delete;
  Swapchain &operator=(Swapchain &&) noexcept;

  [[nodiscard]] explicit operator bool() const;

  enum class Format { Linear, sRGB };

  [[nodiscard]] PixelFormat getPixelFormat() const;
  [[nodiscard]] Extent2D getExtent() const;

  [[nodiscard]] std::size_t getNumBuffers() const;

  [[nodiscard]] const std::vector<Texture> &getBuffers() const;
  [[nodiscard]] const Texture &getBuffer(const uint32_t) const;

  [[nodiscard]] uint32_t getCurrentBufferIndex() const;
  [[nodiscard]] Texture &getCurrentBuffer();

  // ---

  void recreate(const std::optional<VerticalSync> = std::nullopt);

  bool acquireNextImage(const VkSemaphore imageAcquired = VK_NULL_HANDLE);

private:
  Swapchain(const VkInstance, const VkPhysicalDevice, const VkDevice,
            const os::Window &, const Format, const VerticalSync);

  void _createSurface(const os::Window &);

  void _create(const Format, const VerticalSync);
  void _buildBuffers(const Extent2D, const PixelFormat);
  void _destroy();

private:
  VkInstance m_instance{VK_NULL_HANDLE};
  VkPhysicalDevice m_physicalDevice{VK_NULL_HANDLE};
  VkDevice m_device{VK_NULL_HANDLE};

  VkSurfaceKHR m_surface{VK_NULL_HANDLE};
  VkSwapchainKHR m_handle{VK_NULL_HANDLE};

  Format m_format{Format::Linear};
  VerticalSync m_verticalSync{VerticalSync::Enabled};

  std::vector<Texture> m_buffers;
  uint32_t m_currentImageIndex{0};
};

[[nodiscard]] Rect2D getRenderArea(const Swapchain &);

} // namespace rhi
