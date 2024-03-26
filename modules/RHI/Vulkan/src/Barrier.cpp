#include "rhi/Barrier.hpp"
#include "rhi/Buffer.hpp"
#include "rhi/Texture.hpp"

// https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkAccessFlagBits.html

// https://github.com/KhronosGroup/Vulkan-Guide/blob/master/chapters/synchronization.adoc
// https://github.com/KhronosGroup/Vulkan-Docs/wiki/Synchronization-Examples
// https://arm-software.github.io/vulkan_best_practice_for_mobile_developers/samples/performance/pipeline_barriers/pipeline_barriers_tutorial.html

// https://www.lunarg.com/wp-content/uploads/2020/09/Final_LunarG_Guide_to_Vulkan-Synchronization_Validation_08_20.pdf

namespace rhi {

namespace {

void fixAspectMask(VkImageAspectFlags &mask, const Texture &texture) {
  if (mask == VK_IMAGE_ASPECT_NONE) mask = getAspectMask(texture);
}

} // namespace

//
// Barrier class:
//

bool Barrier::isEffective() const {
  return (m_info.memoryBarrierCount + m_info.bufferMemoryBarrierCount +
          m_info.imageMemoryBarrierCount) > 0;
}

Barrier::Barrier(Dependencies &&dependencies)
    : m_dependencies{std::move(dependencies)} {
  m_info = {
    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    .memoryBarrierCount = uint32_t(m_dependencies.memory.size()),
    .pMemoryBarriers = m_dependencies.memory.data(),
    .bufferMemoryBarrierCount = uint32_t(m_dependencies.buffer.size()),
    .pBufferMemoryBarriers = m_dependencies.buffer.data(),
    .imageMemoryBarrierCount = uint32_t(m_dependencies.image.size()),
    .pImageMemoryBarriers = m_dependencies.image.data(),
  };
}

//
// Builder class:
//

// (Sonarlint) Violating S6011 for the sake of readability.

using Builder = Barrier::Builder;

Builder &Builder::memoryBarrier(const BarrierScope &src,
                                const BarrierScope &dst) {
  m_dependencies.memory.emplace_back(VkMemoryBarrier2{
    .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
    .srcStageMask = uint64_t(src.stageMask),
    .srcAccessMask = uint64_t(src.accessMask),
    .dstStageMask = uint64_t(dst.stageMask),
    .dstAccessMask = uint64_t(dst.accessMask),
  });
  return *this;
}
Builder &Builder::bufferBarrier(const BufferInfo &info,
                                const BarrierScope &dst) {
  if (auto &lastScope = info.buffer.m_lastScope; lastScope != dst) {
    m_dependencies.buffer.emplace_back(VkBufferMemoryBarrier2{
      .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
      .srcStageMask = uint64_t(lastScope.stageMask),
      .srcAccessMask = uint64_t(lastScope.accessMask),
      .dstStageMask = uint64_t(dst.stageMask),
      .dstAccessMask = uint64_t(dst.accessMask),
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .buffer = info.buffer.getHandle(),
      .offset = info.offset,
      .size = info.size,
    });
    lastScope = dst;
  }
  return *this;
}
Builder &Builder::imageBarrier(ImageInfo info, const BarrierScope &dst) {
  auto [layout, lastScope] =
    std::tie(info.image.m_layout, info.image.m_lastScope);

  if (layout != info.newLayout || lastScope != dst) {
    fixAspectMask(info.subresourceRange.aspectMask, info.image);
    _imageBarrier(info.image.getImageHandle(), lastScope, dst, layout,
                  info.newLayout, info.subresourceRange);
    layout = info.newLayout;
    lastScope = dst;
  }
  return *this;
}

Barrier Builder::build() { return Barrier{std::move(m_dependencies)}; }

//
// (private):
//

Builder &
Builder::_imageBarrier(const VkImage image, const BarrierScope &src,
                       const BarrierScope &dst, const ImageLayout oldLayout,
                       const ImageLayout newLayout,
                       const VkImageSubresourceRange &subresourceRange) {
  assert(newLayout != ImageLayout::Undefined);
  m_dependencies.image.emplace_back(VkImageMemoryBarrier2{
    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    .srcStageMask = uint64_t(src.stageMask),
    .srcAccessMask = uint64_t(src.accessMask),
    .dstStageMask = uint64_t(dst.stageMask),
    .dstAccessMask = uint64_t(dst.accessMask),
    .oldLayout = VkImageLayout(oldLayout),
    .newLayout = VkImageLayout(newLayout),
    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
    .image = image,
    .subresourceRange = subresourceRange,
  });
  return *this;
}

} // namespace rhi
