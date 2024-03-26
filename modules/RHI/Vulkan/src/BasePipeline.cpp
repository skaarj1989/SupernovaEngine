#include "rhi/BasePipeline.hpp"
#include <cassert>

namespace rhi {

BasePipeline::BasePipeline(BasePipeline &&other) noexcept
    : m_device{other.m_device}, m_layout{std::move(other.m_layout)},
      m_handle{other.m_handle} {
  other.m_device = VK_NULL_HANDLE;
  other.m_handle = VK_NULL_HANDLE;
}
BasePipeline::~BasePipeline() { _destroy(); }

BasePipeline &BasePipeline::operator=(BasePipeline &&rhs) noexcept {
  if (this != &rhs) {
    _destroy();

    std::swap(m_device, rhs.m_device);
    m_layout = std::move(rhs.m_layout);
    std::swap(m_handle, rhs.m_handle);
  }
  return *this;
}

BasePipeline::operator bool() const { return m_handle != VK_NULL_HANDLE; }

VkPipeline BasePipeline::getHandle() const { return m_handle; }

const PipelineLayout &BasePipeline::getLayout() const { return m_layout; }
VkDescriptorSetLayout
BasePipeline::getDescriptorSetLayout(const DescriptorSetIndex index) const {
  return m_layout.getDescriptorSet(index);
}

//
// (private):
//

BasePipeline::BasePipeline(const VkDevice device, PipelineLayout &&layout,
                           const VkPipeline pipeline)
    : m_device{device}, m_layout{std::move(layout)}, m_handle{pipeline} {
  assert(m_device != VK_NULL_HANDLE);
}

void BasePipeline::_destroy() noexcept {
  if (m_handle != VK_NULL_HANDLE) {
    vkDestroyPipeline(m_device, m_handle, nullptr);

    m_device = VK_NULL_HANDLE;
    m_handle = VK_NULL_HANDLE;
  }
}

} // namespace rhi
