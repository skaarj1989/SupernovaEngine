#include "renderer/TransientResources.hpp"
#include "math/Hash.hpp"
#include "spdlog/spdlog.h"

namespace std {

template <> struct hash<gfx::FrameGraphTexture::Desc> {
  auto operator()(const gfx::FrameGraphTexture::Desc &desc) const noexcept {
    size_t h{0};
    hashCombine(h, desc.extent.width, desc.extent.height, desc.depth,
                desc.format, desc.numMipLevels, desc.layers, desc.cubemap,
                desc.usageFlags);
    return h;
  }
};
template <> struct hash<gfx::FrameGraphBuffer::Desc> {
  auto operator()(const gfx::FrameGraphBuffer::Desc &desc) const noexcept {
    size_t h{0};
    hashCombine(h, desc.type, desc.dataSize());
    return h;
  }
};

} // namespace std

namespace gfx {

namespace {

void heartbeat(auto &pool) {
  // A resource's life (for how long it is going to be cached).
  constexpr auto kMaxNumFrames = 10;

  auto &[resources, entryGroups] = pool;

  auto groupsIt = entryGroups.begin();
  while (groupsIt != entryGroups.end()) {
    auto &[_, group] = *groupsIt;
    if (group.empty()) {
      groupsIt = entryGroups.erase(groupsIt);
    } else {
      auto entryIt = group.begin();
      while (entryIt != group.cend()) {
        auto &[resource, life] = *entryIt;
        ++life;
        if (life >= kMaxNumFrames) {
          SPDLOG_TRACE("Deleting resource: {}", fmt::ptr(resource));
          *resource = {};
          entryIt = group.erase(entryIt);
        } else {
          ++entryIt;
        }
      }
      ++groupsIt;
    }
  }

  if (!resources.empty()) {
    ZoneScoped;

    auto [ret, last] =
      std::ranges::remove_if(resources, [](auto &r) { return !(*r); });
    resources.erase(ret, last);
  }
}

} // namespace

//
// TransientResources class:
//

TransientResources::TransientResources(rhi::RenderDevice &rd)
    : m_renderDevice{rd} {}

void TransientResources::update() {
  heartbeat(m_textures);
  heartbeat(m_buffers);
}

rhi::Texture *
TransientResources::acquireTexture(const FrameGraphTexture::Desc &desc) {
  const auto h = std::hash<FrameGraphTexture::Desc>{}(desc);

  if (auto &pool = m_textures.entryGroups[h]; pool.empty()) {
    ZoneScoped;

    auto texture = rhi::Texture::Builder{}
                     .setExtent(desc.extent, desc.depth)
                     .setPixelFormat(desc.format)
                     .setNumMipLevels(desc.numMipLevels > 0
                                        ? std::optional{desc.numMipLevels}
                                        : std::nullopt)
                     .setNumLayers(desc.layers > 0 ? std::optional{desc.layers}
                                                   : std::nullopt)
                     .setUsageFlags(desc.usageFlags)
                     .setCubemap(desc.cubemap)
                     .setupOptimalSampler(false)
                     .build(m_renderDevice);

    m_textures.resources.emplace_back(
      std::make_unique<rhi::Texture>(std::move(texture)));

    auto *ptr = m_textures.resources.back().get();
    SPDLOG_TRACE("Created texture: {}", fmt::ptr(ptr));
    return ptr;
  } else {
    auto *texture = pool.back().resource;
    pool.pop_back();
    return texture;
  }
}
void TransientResources::releaseTexture(const FrameGraphTexture::Desc &desc,
                                        rhi::Texture *texture) {
  const auto h = std::hash<FrameGraphTexture::Desc>{}(desc);
  m_textures.entryGroups[h].emplace_back(texture, 0u);
}

rhi::Buffer *
TransientResources::acquireBuffer(const FrameGraphBuffer::Desc &desc) {
  assert(desc.dataSize() > 0);
  const auto h = std::hash<FrameGraphBuffer::Desc>{}(desc);

  if (auto &pool = m_buffers.entryGroups[h]; pool.empty()) {
    ZoneScoped;

    std::unique_ptr<rhi::Buffer> buffer;
    switch (desc.type) {
      using enum BufferType;

    case UniformBuffer:
      buffer = std::make_unique<rhi::UniformBuffer>(
        m_renderDevice.createUniformBuffer(desc.dataSize()));
      break;
    case StorageBuffer:
      buffer = std::make_unique<rhi::StorageBuffer>(
        m_renderDevice.createStorageBuffer(desc.dataSize()));
      break;

    case VertexBuffer:
      buffer = std::make_unique<rhi::VertexBuffer>(
        m_renderDevice.createVertexBuffer(desc.stride, desc.capacity));
      break;

    default:
      assert(false);
    }
    m_buffers.resources.push_back(std::move(buffer));
    auto *ptr = m_buffers.resources.back().get();
    SPDLOG_TRACE("Created buffer: {}", fmt::ptr(ptr));
    return ptr;
  } else {
    auto *buffer = pool.back().resource;
    pool.pop_back();
    return buffer;
  }
}
void TransientResources::releaseBuffer(const FrameGraphBuffer::Desc &desc,
                                       rhi::Buffer *buffer) {
  const auto h = std::hash<FrameGraphBuffer::Desc>{}(desc);
  m_buffers.entryGroups[h].emplace_back(buffer, 0u);
}

} // namespace gfx
