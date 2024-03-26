#pragma once

#include "rhi/Buffer.hpp"

namespace rhi {

enum class IndexType { Undefined = 0, UInt16 = 2, UInt32 = 4 };

class IndexBuffer final : public Buffer {
  friend class RenderDevice; // Calls the private constructor.

public:
  IndexBuffer() = default;

  [[nodiscard]] IndexType getIndexType() const;
  [[nodiscard]] Stride getStride() const;
  [[nodiscard]] VkDeviceSize getCapacity() const;

private:
  IndexBuffer(Buffer &&, const IndexType);

private:
  IndexType m_indexType{IndexType::Undefined};
};

} // namespace rhi
