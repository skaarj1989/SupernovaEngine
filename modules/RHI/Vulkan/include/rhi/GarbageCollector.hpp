#pragma once

#include "Buffer.hpp"
#include "Texture.hpp"
#include "FrameIndex.hpp"

namespace rhi {

class GarbageCollector {
public:
  void clear();

  void push(Buffer *buffer) { m_buffers.emplace_back(0, buffer); }
  void push(Texture *texture) { m_textures.emplace_back(0, texture); }
  void step(const FrameIndex::ValueType threshold);

private:
  template <class ResourceT>
  using Cache = std::vector<std::pair<FrameIndex::ValueType, ResourceT *>>;
  Cache<Buffer> m_buffers;
  Cache<Texture> m_textures;
};

} // namespace rhi
