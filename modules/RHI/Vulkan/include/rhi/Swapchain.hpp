#pragma once

#include "rhi/Rect2D.hpp"
#include "rhi/Texture.hpp"
#include "os/Window.hpp"

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
  [[nodiscard]] const Texture &getBuffer(uint32_t i) const;

  [[nodiscard]] uint32_t getCurrentBufferIndex() const;
  [[nodiscard]] Texture &getCurrentBuffer();

  // ---

  void recreate(std::optional<VerticalSync> = std::nullopt);

  bool acquireNextImage(VkSemaphore imageAcquired = VK_NULL_HANDLE);

private:
  Swapchain(VkInstance, VkPhysicalDevice, VkDevice, const os::Window &, Format,
            VerticalSync);

  void _createSurface(const os::Window &);

  void _create(Format, VerticalSync);
  void _buildBuffers(Extent2D, PixelFormat);
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
