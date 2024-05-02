#pragma once

#include "TexelFilter.hpp"
#include "ShaderType.hpp"
#include "FramebufferInfo.hpp"
#include "GeometryInfo.hpp"
#include "Barrier.hpp"
#include "DescriptorSetAllocator.hpp"
#include "DescriptorSetBuilder.hpp"

#include "DebugMarker.hpp"

#include "glm/ext/vector_uint3.hpp"

#include "tracy/Tracy.hpp"
#include "tracy/TracyVulkan.hpp"

#include <span>

namespace rhi {

class RenderDevice;
class VertexBuffer;
class IndexBuffer;
class Texture;
class BasePipeline;
class ComputePipeline;

class CommandBuffer final {
  friend class RenderDevice; // Calls the private constructor.
  friend class DebugMarker;  // Calls private _{push/pop}DebugGroup.

  enum class State { Invalid = -1, Initial, Recording, Executable, Pending };

  enum class InvariantFlags {
    None = 0,
    ValidPipeline = 1 << 1,

    GraphicsPipeline = 1 << 2,
    ValidGraphicsPipeline = ValidPipeline | GraphicsPipeline,

    ComputePipeline = 1 << 3,
    ValidComputePipeline = ValidPipeline | ComputePipeline,

    InsideRenderPass = 1 << 4,
    OutsideRenderPass = 1 << 5,
  };

public:
  CommandBuffer();
  CommandBuffer(const CommandBuffer &) = delete;
  CommandBuffer(CommandBuffer &&) noexcept;
  ~CommandBuffer();

  CommandBuffer &operator=(const CommandBuffer &) = delete;
  CommandBuffer &operator=(CommandBuffer &&) noexcept;

  [[nodiscard]] VkCommandBuffer getHandle() const;
  [[nodiscard]] TracyVkCtx getTracyContext() const;

  Barrier::Builder &getBarrierBuilder();
  [[nodiscard]] DescriptorSetBuilder createDescriptorSetBuilder();

  CommandBuffer &begin();
  CommandBuffer &end();
  CommandBuffer &reset();

  // ---

  CommandBuffer &bindPipeline(const BasePipeline &);

  CommandBuffer &dispatch(const ComputePipeline &, const glm::uvec3 &);
  CommandBuffer &dispatch(const glm::uvec3 &);

  CommandBuffer &bindDescriptorSet(const DescriptorSetIndex,
                                   const VkDescriptorSet);

  CommandBuffer &pushConstants(const ShaderStages, const uint32_t offset,
                               const uint32_t size, const void *data);
  template <typename T>
  CommandBuffer &pushConstants(const ShaderStages shaderStages,
                               const uint32_t offset, const T *v) {
    return pushConstants(shaderStages, offset, sizeof(T), v);
  }

  // ---

  // Does not insert barriers for attachments.
  CommandBuffer &beginRendering(const FramebufferInfo &);
  CommandBuffer &endRendering();

  CommandBuffer &setViewport(const Rect2D &);
  CommandBuffer &setScissor(const Rect2D &);

  CommandBuffer &draw(const GeometryInfo &, const uint32_t numInstances = 1);
  CommandBuffer &drawFullScreenTriangle();
  CommandBuffer &drawCube();

  // ---

  CommandBuffer &clear(Buffer &, const uint32_t value = 0);
  // Texture image must be created with TRANSFER_DST.
  CommandBuffer &clear(Texture &, const ClearValue &);

  CommandBuffer &copyBuffer(const Buffer &src, Buffer &dst,
                            const VkBufferCopy &);
  CommandBuffer &copyBuffer(const Buffer &src, Texture &dst);
  // Inserts layout transition barrier for dst.
  CommandBuffer &copyBuffer(const Buffer &src, Texture &dst,
                            std::span<const VkBufferImageCopy>);
  CommandBuffer &copyImage(const Texture &src, const Buffer &dst,
                           const rhi::ImageAspect);

  CommandBuffer &update(Buffer &, const VkDeviceSize offset,
                        const VkDeviceSize size, const void *data);

  CommandBuffer &blit(Texture &src, Texture &dst, const VkFilter);

  CommandBuffer &generateMipmaps(Texture &,
                                 const TexelFilter = TexelFilter::Linear);

  // ---

  [[deprecated("Find the issue and tighten the barrier!")]] CommandBuffer &
  insertFatBarrier();
  CommandBuffer &flushBarriers();

private:
  CommandBuffer(const VkDevice, const VkCommandPool, const VkCommandBuffer,
                const TracyVkCtx, const VkFence);

  [[nodiscard]] bool
  _invariant(const State requiredState,
             const InvariantFlags = InvariantFlags::None) const;

  void _destroy() noexcept;

  void _chunkedUpdate(const VkBuffer, VkDeviceSize offset, VkDeviceSize size,
                      const void *data);

  void _setVertexBuffer(const VertexBuffer *, const VkDeviceSize offset);
  void _setIndexBuffer(const IndexBuffer *);

  void _pushDebugGroup(const std::string_view);
  void _popDebugGroup();

private:
  VkDevice m_device{VK_NULL_HANDLE};
  VkCommandPool m_commandPool{VK_NULL_HANDLE};

  State m_state{State::Invalid};

  VkCommandBuffer m_handle{VK_NULL_HANDLE};
  TracyVkCtx m_tracyContext{nullptr};

  VkFence m_fence{VK_NULL_HANDLE};

  DescriptorSetAllocator m_descriptorSetAllocator;
  DescriptorSetCache m_descriptorSetCache;

  Barrier::Builder m_barrierBuilder;

  const BasePipeline *m_pipeline{nullptr};
  const VertexBuffer *m_vertexBuffer{nullptr};
  const IndexBuffer *m_indexBuffer{nullptr};

  bool m_insideRenderPass{false};
};

void prepareForAttachment(CommandBuffer &, const Texture &,
                          const bool readOnly);
void prepareForReading(CommandBuffer &, const Texture &);

} // namespace rhi

template <>
struct has_flags<rhi::CommandBuffer::InvariantFlags> : std::true_type {};

#define _TRACY_GPU_ZONE(TracyContext, CommandBufferHandle, Label)              \
  ZoneScopedN(Label);                                                          \
  TracyVkZone(TracyContext, CommandBufferHandle, Label)

#define TRACY_GPU_ZONE(CommandBuffer, Label)                                   \
  _TRACY_GPU_ZONE(CommandBuffer.getTracyContext(), CommandBuffer.getHandle(),  \
                  Label)

#define TRACY_GPU_TRANSIENT_ZONE(CommandBuffer, Label)                         \
  ZoneTransientN(_tracy_zone, Label, true);                                    \
  TracyVkZoneTransient(CommandBuffer.getTracyContext(), _tracy_vk_zone,        \
                       CommandBuffer.getHandle(), Label, true)

#define RHI_GPU_ZONE(CommandBuffer, Label)                                     \
  RHI_NAMED_DEBUG_MARKER(CommandBuffer, Label);                                \
  TRACY_GPU_TRANSIENT_ZONE(CommandBuffer, Label)
