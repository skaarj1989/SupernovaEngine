#pragma once

#include <string>

namespace rhi {
class Buffer;
}

namespace gfx {

enum class BufferType {
  UniformBuffer,
  StorageBuffer,
  VertexBuffer,
  IndexBuffer,
};

class FrameGraphBuffer {
public:
  struct Desc {
    BufferType type;
    uint32_t stride{sizeof(std::byte)};
    uint64_t capacity;

    [[nodiscard]] constexpr auto dataSize() const { return stride * capacity; }
  };

  void create(const Desc &, void *allocator);
  void destroy(const Desc &, void *allocator);

  void preRead(const Desc &, uint32_t flags, void *context);
  void preWrite(const Desc &, uint32_t flags, void *context);

  [[nodiscard]] static std::string toString(const Desc &);

  rhi::Buffer *buffer{nullptr};
};

} // namespace gfx
