#include "rhi/FrameController.hpp"

// https://www.intel.com/content/www/us/en/developer/articles/training/practical-approach-to-vulkan-part-1.html

namespace rhi {

FrameController::FrameController(RenderDevice &rd, Swapchain &swapchain,
                                 uint32_t framesInFlight)
    : m_renderDevice{&rd}, m_swapchain{&swapchain},
      m_frameIndex{framesInFlight} {
  _create(framesInFlight);
}
FrameController::FrameController(FrameController &&other) noexcept
    : m_renderDevice{other.m_renderDevice}, m_swapchain{other.m_swapchain},
      m_frames{std::move(other.m_frames)}, m_frameIndex{other.m_frameIndex} {
  other.m_renderDevice = nullptr;
  other.m_swapchain = nullptr;
  other.m_frameIndex = FrameIndex{0};
}
FrameController::~FrameController() { _destroy(); }

FrameController &FrameController::operator=(FrameController &&rhs) noexcept {
  if (this != &rhs) {
    _destroy();

    std::swap(m_renderDevice, rhs.m_renderDevice);
    std::swap(m_swapchain, rhs.m_swapchain);
    std::swap(m_frames, rhs.m_frames);
    std::swap(m_frameIndex, rhs.m_frameIndex);
  }
  return *this;
}

FrameController::operator bool() const {
  return m_swapchain != nullptr && *m_swapchain && !m_frames.empty();
}

RenderTargetView FrameController::getCurrentTarget() const {
  assert(m_swapchain);
  return {
    m_frameIndex,
    m_swapchain->getCurrentBuffer(),
  };
}

CommandBuffer &FrameController::beginFrame() {
  assert(m_swapchain);
  auto &[cb, imageAcquired, _] = m_frames[m_frameIndex];

  cb.reset();
  m_swapchain->acquireNextImage(imageAcquired);
  return cb.begin();
}
FrameController &FrameController::endFrame() {
  assert(m_swapchain);
  auto &[cb, imageAcquired, renderCompleted] = m_frames[m_frameIndex];

  cb.getBarrierBuilder().imageBarrier(
    {
      .image = m_swapchain->getCurrentBuffer(),
      .newLayout = ImageLayout::Present,
      .subresourceRange = {.levelCount = 1, .layerCount = 1},
    },
    {
      .stageMask = PipelineStages::Bottom,
      .accessMask = Access::None,
    });

  m_renderDevice->execute(
    cb, JobInfo{
          .wait = imageAcquired,
          .waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
          .signal = renderCompleted,
        });
  return *this;
}

void FrameController::present() {
  assert(m_swapchain);
  auto &currentFrame = m_frames[m_frameIndex];
  m_renderDevice->present(*m_swapchain, currentFrame.renderCompleted);
  ++m_frameIndex;
}

void FrameController::recreate() { m_swapchain->recreate(); }

//
// (private):
//

void FrameController::_create(uint32_t framesInFlight) {
  m_frames.reserve(framesInFlight);
  std::generate_n(std::back_inserter(m_frames), framesInFlight,
                  [&rd = *m_renderDevice] {
                    return PerFrameData{
                      .commandBuffer = rd.createCommandBuffer(),
                      .imageAcquired = rd.createSemaphore(),
                      .renderCompleted = rd.createSemaphore(),
                    };
                  });
}
void FrameController::_destroy() noexcept {
  if (!m_renderDevice) return;

  for (auto &f : m_frames) {
    f.commandBuffer = {};
    m_renderDevice->destroy(f.imageAcquired).destroy(f.renderCompleted);
  }
  m_frames.clear();

  m_swapchain = nullptr;
  m_renderDevice = nullptr;
}

} // namespace rhi
