#pragma once

#include "rhi/VertexBuffer.hpp"
#include "rhi/IndexBuffer.hpp"
#include "rhi/TexelFilter.hpp"
#include "rhi/Texture.hpp"
#include "rhi/ComputePipeline.hpp"
#include "rhi/GraphicsPipeline.hpp"
#include "rhi/FramebufferInfo.hpp"
#include "rhi/GeometryInfo.hpp"
#include "rhi/Barrier.hpp"
#include "rhi/DescriptorPool.hpp"
#include "rhi/DescriptorSetBuilder.hpp"
#include "tracy/Tracy.hpp"
#include "tracy/TracyVulkan.hpp"

namespace rhi {

class DebugMarker;

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

  CommandBuffer &bindDescriptorSet(uint32_t set, VkDescriptorSet);

  CommandBuffer &pushConstants(ShaderStages, uint32_t offset, uint32_t size,
                               const void *data);
  template <typename T>
  CommandBuffer &pushConstants(ShaderStages shaderStages, uint32_t offset,
                               const T *v) {
    return pushConstants(shaderStages, offset, sizeof(T), v);
  }

  // ---

  // Does not insert barriers for attachments.
  CommandBuffer &beginRendering(const FramebufferInfo &);
  CommandBuffer &endRendering();

  CommandBuffer &setViewport(const Rect2D &);
  CommandBuffer &setScissor(const Rect2D &);

  CommandBuffer &draw(const GeometryInfo &, uint32_t numInstances = 1);
  CommandBuffer &drawFullScreenTriangle();
  CommandBuffer &drawCube();

  // ---

  CommandBuffer &clear(Buffer &);
  // Texture image must be created with TRANSFER_DST.
  CommandBuffer &clear(Texture &, const ClearValue &);

  CommandBuffer &copyBuffer(const Buffer &src, Buffer &dst,
                            const VkBufferCopy &);
  CommandBuffer &copyBuffer(const Buffer &src, Texture &dst);
  // Inserts layout transition barrier for dst.
  CommandBuffer &copyBuffer(const Buffer &src, Texture &dst,
                            std::span<const VkBufferImageCopy>);

  CommandBuffer &update(Buffer &, VkDeviceSize offset, VkDeviceSize size,
                        const void *data);

  CommandBuffer &blit(Texture &src, Texture &dst, VkFilter);

  CommandBuffer &generateMipmaps(Texture &, TexelFilter = TexelFilter::Linear);

  // ---

  [[deprecated("Find the issue and tighten the barrier!")]] CommandBuffer &
  insertFatBarrier();
  CommandBuffer &flushBarriers();

private:
  CommandBuffer(VkDevice, VkCommandPool, VkCommandBuffer, TracyVkCtx, VkFence);

  [[nodiscard]] bool _invariant(State requiredState,
                                InvariantFlags = InvariantFlags::None) const;

  void _destroy() noexcept;

  void _chunkedUpdate(VkBuffer, VkDeviceSize offset, VkDeviceSize size,
                      const void *data);

  void _setVertexBuffer(const VertexBuffer *, VkDeviceSize offset);
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

  DescriptorPool m_descriptorPool;
  DescriptorSetCache m_descriptorSetCache;

  Barrier::Builder m_barrierBuilder;

  const BasePipeline *m_pipeline{nullptr};
  const VertexBuffer *m_vertexBuffer{nullptr};
  const IndexBuffer *m_indexBuffer{nullptr};

  bool m_insideRenderPass{false};
};
template <> struct has_flags<CommandBuffer::InvariantFlags> : std::true_type {};

void prepareForAttachment(CommandBuffer &, const Texture &, bool readOnly);
void prepareForReading(CommandBuffer &, const Texture &);

} // namespace rhi
