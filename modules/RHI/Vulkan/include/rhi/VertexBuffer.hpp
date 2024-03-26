#pragma once

#include "Buffer.hpp"

namespace rhi {

class VertexBuffer final : public Buffer {
  friend class RenderDevice; // Calls the private constructor.

public:
  VertexBuffer() = default;

  [[nodiscard]] Stride getStride() const;
  [[nodiscard]] VkDeviceSize getCapacity() const;

private:
  VertexBuffer(Buffer &&, const Stride);

private:
  Stride m_stride{0};
};

} // namespace rhi
