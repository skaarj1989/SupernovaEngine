#pragma once

#include "rhi/Buffer.hpp"

namespace rhi {

class VertexBuffer final : public Buffer {
  friend class RenderDevice; // Calls the private constructor.

public:
  VertexBuffer() = default;

  [[nodiscard]] uint32_t getStride() const;
  [[nodiscard]] VkDeviceSize getCapacity() const;

private:
  VertexBuffer(Buffer &&, uint32_t stride);

private:
  uint32_t m_stride{0};
};

} // namespace rhi
