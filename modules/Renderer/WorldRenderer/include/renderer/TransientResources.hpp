#pragma once

#include "FrameGraphTexture.hpp"
#include "FrameGraphBuffer.hpp"
#include "robin_hood.h"
#include <memory>
#include <vector>

namespace rhi {
class RenderDevice;
}

namespace gfx {

class TransientResources {
public:
  explicit TransientResources(rhi::RenderDevice &);
  TransientResources(const TransientResources &) = delete;
  TransientResources(TransientResources &&) noexcept = delete;
  ~TransientResources() = default;

  TransientResources &operator=(const TransientResources &) = delete;
  TransientResources &operator=(TransientResources &&) noexcept = delete;

  void update();

  [[nodiscard]] rhi::Texture *acquireTexture(const FrameGraphTexture::Desc &);
  void releaseTexture(const FrameGraphTexture::Desc &, rhi::Texture *);

  [[nodiscard]] rhi::Buffer *acquireBuffer(const FrameGraphBuffer::Desc &);
  void releaseBuffer(const FrameGraphBuffer::Desc &, rhi::Buffer *);

private:
  rhi::RenderDevice &m_renderDevice;

  template <class T> struct Pool {
    using HashType = std::size_t;

    struct Entry {
      T *resource;
      std::size_t life{0}; // In frames.
    };
    using Entries = std::vector<Entry>;
    using EntryGroups = robin_hood::unordered_map<HashType, Entries>;

    std::vector<std::unique_ptr<T>> resources;
    EntryGroups entryGroups;
  };
  Pool<rhi::Texture> m_textures;
  Pool<rhi::Buffer> m_buffers;
};

} // namespace gfx
