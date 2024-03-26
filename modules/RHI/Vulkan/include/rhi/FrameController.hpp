#pragma once

#include "CommandBuffer.hpp"
#include "RenderTargetView.hpp"

namespace rhi {

class RenderDevice;
class Swapchain;

// Simplifies usage of frames in flight.
class FrameController final {
public:
  FrameController() = default;
  FrameController(RenderDevice &, Swapchain &,
                  const FrameIndex::ValueType numFramesInFlight);
  FrameController(const FrameController &) = delete;
  FrameController(FrameController &&) noexcept;
  ~FrameController();

  FrameController &operator=(const FrameController &) = delete;
  FrameController &operator=(FrameController &&) noexcept;

  [[nodiscard]] explicit operator bool() const;

  [[nodiscard]] FrameIndex::ValueType size() const;
  [[nodiscard]] RenderTargetView getCurrentTarget() const;

  CommandBuffer &beginFrame();
  FrameController &endFrame();

  void present();

  void recreate();

private:
  void _create(const FrameIndex::ValueType numFramesInFlight);
  void _destroy() noexcept;

private:
  RenderDevice *m_renderDevice{nullptr};
  Swapchain *m_swapchain{nullptr};

  struct PerFrameData {
    CommandBuffer commandBuffer;
    VkSemaphore imageAcquired{VK_NULL_HANDLE};
    VkSemaphore renderCompleted{VK_NULL_HANDLE};
  };
  std::vector<PerFrameData> m_frames;
  FrameIndex m_frameIndex;
};

} // namespace rhi
