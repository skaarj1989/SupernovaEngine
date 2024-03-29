#include "BuildPropertyBuffer.hpp"
#include "renderer/Material.hpp"
#include "glm/common.hpp" // ceil, max
#include "tracy/Tracy.hpp"
#include <cstring> // memcpy

namespace gfx {

VkDeviceSize adjustStride(const VkDeviceSize stride,
                          const VkDeviceSize minOffsetAlignment) {
  assert(stride > 0 && minOffsetAlignment > 0);
  return static_cast<VkDeviceSize>(
           glm::ceil(float(glm::max(stride, minOffsetAlignment)) /
                     float(minOffsetAlignment))) *
         minOffsetAlignment;
}

std::vector<std::byte>
buildPropertyBuffer(const PropertyLayout &layout,
                    const std::vector<Property> &properties,
                    const VkDeviceSize minOffsetAlignment) {
  ZoneScopedN("BuildPropertyBuffer");

  const auto bufferSize = adjustStride(layout.stride, minOffsetAlignment);
  std::vector<std::byte> buffer(bufferSize);
  for (auto i = 0; i < layout.members.size(); ++i) {
    const auto [offset, size] = layout.members[i];
    std::memcpy(&buffer[offset], &properties[i].value, size);
  }
  return buffer;
}

} // namespace gfx
