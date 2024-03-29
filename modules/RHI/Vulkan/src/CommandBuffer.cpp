#include "rhi/CommandBuffer.hpp"
#include "VisitorHelper.hpp"

#include "rhi/IndexBuffer.hpp"
#include "rhi/VertexBuffer.hpp"
#include "rhi/Texture.hpp"
#include "rhi/ComputePipeline.hpp"
#include "VkCheck.hpp"

#include "glm/gtc/type_ptr.hpp" // value_ptr

// https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#commandbuffers-lifecycle

namespace rhi {

namespace {

constexpr VkDeviceSize kMaxDataSize{65536};

[[nodiscard]] auto toVk(const IndexType indexType) {
  switch (indexType) {
  case IndexType::UInt16:
    return VK_INDEX_TYPE_UINT16;
  case IndexType::UInt32:
    return VK_INDEX_TYPE_UINT32;
  }
  assert(false);
  return VK_INDEX_TYPE_MAX_ENUM;
}

[[nodiscard]] auto toVk(const ClearValue &clearValue) {
  return std::visit(
    Overload{
      [](const glm::vec4 &v) {
        VkClearValue result{};
        // Sonarlint is wrong (S3519)
        // The following .color is a float[4] array (same size as vec4).
        std::memcpy(&result.color.float32, glm::value_ptr(v),
                    sizeof(glm::vec4));
        return result;
      },
      [](const float v) { return VkClearValue{.depthStencil = {.depth = v}}; },
      [](const uint32_t v) {
        return VkClearValue{.depthStencil = {.stencil = v}};
      },
    },
    clearValue);
}
[[nodiscard]] VkRenderingAttachmentInfo toVk(const AttachmentInfo &attachment,
                                             const bool readOnly) {
  auto &[target, layer, face, clearValue] = attachment;
  assert(!readOnly || !clearValue.has_value());
  return {
    .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
    .imageView =
      layer ? target->getLayer(*layer, face) : target->getImageView(),
    .imageLayout = VkImageLayout(target->getImageLayout()),
    .resolveMode = VK_RESOLVE_MODE_NONE,
    .loadOp =
      clearValue ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
    .storeOp =
      readOnly ? VK_ATTACHMENT_STORE_OP_NONE : VK_ATTACHMENT_STORE_OP_STORE,
    .clearValue = clearValue ? toVk(*clearValue) : VkClearValue{},
  };
}

} // namespace

#define _TRACY_GPU_ZONE2(Label)                                                \
  _TRACY_GPU_ZONE(m_tracyContext, m_handle, "RHI::" Label)

//
// CommandBuffer class:
//

CommandBuffer::CommandBuffer() { m_descriptorSetCache.reserve(100); }

CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept
    : m_device{other.m_device}, m_commandPool{other.m_commandPool},
      m_state{other.m_state}, m_handle{other.m_handle},
      m_tracyContext{other.m_tracyContext}, m_fence{other.m_fence},
      m_descriptorSetAllocator{std::move(other.m_descriptorSetAllocator)},
      m_descriptorSetCache{std::move(other.m_descriptorSetCache)},
      m_barrierBuilder{std::move(other.m_barrierBuilder)},
      m_pipeline{other.m_pipeline}, m_vertexBuffer{other.m_vertexBuffer},
      m_indexBuffer{other.m_indexBuffer},
      m_insideRenderPass{other.m_insideRenderPass} {
  other.m_device = VK_NULL_HANDLE;
  other.m_commandPool = VK_NULL_HANDLE;

  other.m_state = State::Invalid;

  other.m_handle = VK_NULL_HANDLE;
  other.m_tracyContext = nullptr;

  other.m_fence = VK_NULL_HANDLE;

  other.m_pipeline = nullptr;
  other.m_vertexBuffer = nullptr;
  other.m_indexBuffer = nullptr;

  other.m_insideRenderPass = false;
}
CommandBuffer::~CommandBuffer() { _destroy(); }

CommandBuffer &CommandBuffer::operator=(CommandBuffer &&rhs) noexcept {
  if (this != &rhs) {
    _destroy();

    std::swap(m_device, rhs.m_device);
    std::swap(m_commandPool, rhs.m_commandPool);

    std::swap(m_state, rhs.m_state);

    std::swap(m_handle, rhs.m_handle);
    std::swap(m_tracyContext, rhs.m_tracyContext);

    std::swap(m_fence, rhs.m_fence);

    std::swap(m_descriptorSetAllocator, rhs.m_descriptorSetAllocator);
    std::swap(m_descriptorSetCache, rhs.m_descriptorSetCache);

    std::swap(m_barrierBuilder, rhs.m_barrierBuilder);

    std::swap(m_pipeline, rhs.m_pipeline);
    std::swap(m_vertexBuffer, rhs.m_vertexBuffer);
    std::swap(m_indexBuffer, rhs.m_indexBuffer);

    std::swap(m_insideRenderPass, rhs.m_insideRenderPass);
  }
  return *this;
}

VkCommandBuffer CommandBuffer::getHandle() const { return m_handle; }
TracyVkCtx CommandBuffer::getTracyContext() const { return m_tracyContext; }

Barrier::Builder &CommandBuffer::getBarrierBuilder() {
  return m_barrierBuilder;
}

DescriptorSetBuilder CommandBuffer::createDescriptorSetBuilder() {
  return DescriptorSetBuilder{m_device, m_descriptorSetAllocator,
                              m_descriptorSetCache};
}

CommandBuffer &CommandBuffer::begin() {
  assert(_invariant(State::Initial));

  VK_CHECK(vkResetFences(m_device, 1, &m_fence));
  const VkCommandBufferBeginInfo beginInfo{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };
  VK_CHECK(vkBeginCommandBuffer(m_handle, &beginInfo));

  m_state = State::Recording;
  return *this;
}
CommandBuffer &CommandBuffer::end() {
  assert(_invariant(State::Recording, InvariantFlags::OutsideRenderPass));

  TracyVkCollect(m_tracyContext, m_handle);
  VK_CHECK(vkEndCommandBuffer(m_handle));

  m_state = State::Executable;

  m_pipeline = nullptr;
  m_vertexBuffer = nullptr;
  m_indexBuffer = nullptr;

  return *this;
}

CommandBuffer &CommandBuffer::reset() {
  assert(m_state != State::Recording);

  if (m_state == State::Pending) {
    assert(m_handle != VK_NULL_HANDLE);

    VK_CHECK(vkWaitForFences(m_device, 1, &m_fence, VK_TRUE, UINT64_MAX));
    VK_CHECK(vkResetCommandBuffer(
      m_handle, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

    m_descriptorSetCache.clear();
    m_descriptorSetAllocator.reset();

    m_state = State::Initial;
  }
  return *this;
}

CommandBuffer &CommandBuffer::bindPipeline(const BasePipeline &pipeline) {
  assert(pipeline);
  assert(_invariant(State::Recording));

  if (m_pipeline != &pipeline) {
    _TRACY_GPU_ZONE2("BindPipeline");
    vkCmdBindPipeline(m_handle, pipeline.getBindPoint(), pipeline.getHandle());
    m_pipeline = std::addressof(pipeline);
  }
  return *this;
}

CommandBuffer &CommandBuffer::dispatch(const ComputePipeline &pipeline,
                                       const glm::uvec3 &groupCount) {
  return bindPipeline(pipeline).dispatch(groupCount);
}
CommandBuffer &CommandBuffer::dispatch(const glm::uvec3 &groupCount) {
  assert(_invariant(State::Recording, InvariantFlags::ValidComputePipeline));

  _TRACY_GPU_ZONE2("Dispatch");
  flushBarriers();
  vkCmdDispatch(m_handle, groupCount.x, groupCount.y, groupCount.z);
  return *this;
}
CommandBuffer &
CommandBuffer::bindDescriptorSet(const DescriptorSetIndex index,
                                 const VkDescriptorSet descriptorSet) {
  assert(descriptorSet != VK_NULL_HANDLE);
  assert(_invariant(State::Recording, InvariantFlags::ValidPipeline));

  _TRACY_GPU_ZONE2("BindDescriptorSet");
  vkCmdBindDescriptorSets(m_handle, m_pipeline->getBindPoint(),
                          m_pipeline->getLayout().getHandle(), index, 1,
                          &descriptorSet, 0, nullptr);
  return *this;
}

CommandBuffer &CommandBuffer::pushConstants(const ShaderStages shaderStages,
                                            const uint32_t offset,
                                            const uint32_t size,
                                            const void *data) {
  assert(data && size > 0);
  assert(_invariant(State::Recording, InvariantFlags::ValidPipeline));

  _TRACY_GPU_ZONE2("PushConstants");
  vkCmdPushConstants(m_handle, m_pipeline->getLayout().getHandle(),
                     std::to_underlying(shaderStages), offset, size, data);
  return *this;
}

CommandBuffer &
CommandBuffer::beginRendering(const FramebufferInfo &framebufferInfo) {
  assert(_invariant(State::Recording, InvariantFlags::OutsideRenderPass));

  _TRACY_GPU_ZONE2("BeginRendering");
  VkRenderingAttachmentInfo depthAttachment{};
  if (framebufferInfo.depthAttachment) {
    depthAttachment =
      toVk(*framebufferInfo.depthAttachment, framebufferInfo.depthReadOnly);
  }
  VkRenderingAttachmentInfo stencilAttachment{};
  if (framebufferInfo.stencilAttachment) {
    stencilAttachment =
      toVk(*framebufferInfo.stencilAttachment, framebufferInfo.stencilReadOnly);
  }
  std::vector<VkRenderingAttachmentInfo> colorAttachments;
  colorAttachments.reserve(framebufferInfo.colorAttachments.size());
  for (const auto &attachment : framebufferInfo.colorAttachments) {
    colorAttachments.emplace_back(toVk(attachment, false));
  }

  const VkRenderingInfo renderingInfo{
    .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
    .renderArea = static_cast<VkRect2D>(framebufferInfo.area),
    .layerCount = framebufferInfo.layers,
    .colorAttachmentCount = uint32_t(colorAttachments.size()),
    .pColorAttachments = colorAttachments.data(),
    .pDepthAttachment = depthAttachment.imageView ? &depthAttachment : nullptr,
    .pStencilAttachment =
      stencilAttachment.imageView ? &stencilAttachment : nullptr,
  };

  flushBarriers();
  vkCmdBeginRendering(m_handle, &renderingInfo);

  m_insideRenderPass = true;

  return setViewport(framebufferInfo.area).setScissor(framebufferInfo.area);
}
CommandBuffer &CommandBuffer::endRendering() {
  assert(_invariant(State::Recording, InvariantFlags::InsideRenderPass));

  _TRACY_GPU_ZONE2("EndRendering");
  vkCmdEndRendering(m_handle);
  m_insideRenderPass = false;

  return *this;
}

CommandBuffer &CommandBuffer::setViewport(const Rect2D &rect) {
  assert(_invariant(State::Recording));

  _TRACY_GPU_ZONE2("SetViewport");
  const VkViewport viewport{
    .x = float(rect.offset.x),
    .y = float(rect.offset.y),
    .width = float(rect.extent.width),
    .height = float(rect.extent.height),
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  vkCmdSetViewport(m_handle, 0, 1, &viewport);
  return *this;
}
CommandBuffer &CommandBuffer::setScissor(const Rect2D &rect) {
  assert(_invariant(State::Recording));

  _TRACY_GPU_ZONE2("SetScissor");
  const auto scissor = static_cast<VkRect2D>(rect);
  vkCmdSetScissor(m_handle, 0, 1, &scissor);
  return *this;
}

CommandBuffer &CommandBuffer::draw(const GeometryInfo &gi,
                                   const uint32_t numInstances) {
  assert(_invariant(State::Recording, InvariantFlags::ValidGraphicsPipeline |
                                        InvariantFlags::InsideRenderPass));
  _TRACY_GPU_ZONE2("Draw");

  constexpr auto kFirstInstance = 0u;
  _setVertexBuffer(gi.vertexBuffer, 0);
  if (gi.indexBuffer && gi.numIndices > 0) {
    _setIndexBuffer(gi.indexBuffer);
    vkCmdDrawIndexed(m_handle, gi.numIndices, numInstances, gi.indexOffset,
                     gi.vertexOffset, kFirstInstance);
  } else {
    assert(gi.numVertices > 0);
    vkCmdDraw(m_handle, gi.numVertices, numInstances, gi.vertexOffset,
              kFirstInstance);
  }
  return *this;
}
CommandBuffer &CommandBuffer::drawFullScreenTriangle() {
  // Triggers the following VVL Warning:
  // BestPractices-DrawState-VtxIndexOutOfBounds
  return draw({.numVertices = 3});
}
CommandBuffer &CommandBuffer::drawCube() { return draw({.numVertices = 36}); }

CommandBuffer &CommandBuffer::clear(Buffer &buffer) {
  assert(buffer);
  assert(_invariant(State::Recording, InvariantFlags::OutsideRenderPass));

  _TRACY_GPU_ZONE2("ClearBuffer");
  flushBarriers();
  vkCmdFillBuffer(m_handle, buffer.getHandle(), 0, VK_WHOLE_SIZE, 0);
  return *this;
}
CommandBuffer &CommandBuffer::clear(Texture &texture,
                                    const ClearValue &clearValue) {
  assert(texture && bool(texture.getUsageFlags() & ImageUsage::TransferDst));
  assert(_invariant(State::Recording, InvariantFlags::OutsideRenderPass));

  _TRACY_GPU_ZONE2("ClearTexture");

  const auto imageHandle = texture.getImageHandle();
  const auto imageLayout = VkImageLayout(texture.getImageLayout());
  const auto v = toVk(clearValue);
  const VkImageSubresourceRange range{
    .aspectMask = getAspectMask(texture),
    .levelCount = VK_REMAINING_MIP_LEVELS,
    .layerCount = VK_REMAINING_ARRAY_LAYERS,
  };

  flushBarriers();
  if (range.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
    vkCmdClearColorImage(m_handle, imageHandle, imageLayout, &v.color, 1,
                         &range);
  } else {
    vkCmdClearDepthStencilImage(m_handle, imageHandle, imageLayout,
                                &v.depthStencil, 1, &range);
  }
  return *this;
}

CommandBuffer &CommandBuffer::copyBuffer(const Buffer &src, Buffer &dst,
                                         const VkBufferCopy &copyRegion) {
  assert(src && dst);
  assert(_invariant(State::Recording, InvariantFlags::OutsideRenderPass));

  _TRACY_GPU_ZONE2("CopyBuffer");
  flushBarriers();
  vkCmdCopyBuffer(m_handle, src.getHandle(), dst.getHandle(), 1, &copyRegion);
  return *this;
}
CommandBuffer &CommandBuffer::copyBuffer(const Buffer &src, Texture &dst) {
  const auto extent = dst.getExtent();
  const auto copyRegion = std::array{
    VkBufferImageCopy{
      .imageSubresource =
        {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .layerCount = 1,
        },
      .imageExtent =
        {
          .width = extent.width,
          .height = extent.height,
          .depth = 1,
        },
    },
  };
  return copyBuffer(src, dst, copyRegion);
}

CommandBuffer &
CommandBuffer::copyBuffer(const Buffer &src, Texture &dst,
                          std::span<const VkBufferImageCopy> copyRegions) {
  assert(src && dst && !copyRegions.empty());
  assert(_invariant(State::Recording, InvariantFlags::OutsideRenderPass));

  _TRACY_GPU_ZONE2("Buffer->Texture");

  constexpr auto kExpectedLayout = ImageLayout::TransferDst;
  m_barrierBuilder.imageBarrier(
    {
      .image = dst, .newLayout = kExpectedLayout,
      /* Default subresourceRange argument = all levels and layers */
    },
    {
      .stageMask = PipelineStages::Transfer,
      .accessMask = Access::TransferWrite,
    });
  flushBarriers();
  vkCmdCopyBufferToImage(m_handle, src.getHandle(), dst.getImageHandle(),
                         VkImageLayout(kExpectedLayout),
                         uint32_t(copyRegions.size()), copyRegions.data());
  return *this;
}

CommandBuffer &CommandBuffer::update(Buffer &buffer, const VkDeviceSize offset,
                                     const VkDeviceSize size,
                                     const void *data) {
  assert(buffer && data);
  assert(_invariant(State::Recording, InvariantFlags::OutsideRenderPass));

  _TRACY_GPU_ZONE2("UpdateBuffer");
  flushBarriers();
  if (size > kMaxDataSize) {
    _chunkedUpdate(buffer.getHandle(), offset, size, data);
  } else {
    vkCmdUpdateBuffer(m_handle, buffer.getHandle(), offset, size, data);
  }
  return *this;
}

CommandBuffer &CommandBuffer::blit(Texture &src, Texture &dst,
                                   const VkFilter filter) {
  assert(src && bool(src.getUsageFlags() & ImageUsage::TransferSrc));
  const auto aspectMask = getAspectMask(dst);
  assert(getAspectMask(src) == aspectMask);
  assert(dst && bool(dst.getUsageFlags() & ImageUsage::TransferDst));
  assert(_invariant(State::Recording, InvariantFlags::OutsideRenderPass));

  _TRACY_GPU_ZONE2("Texture->Texture");
  getBarrierBuilder()
    .imageBarrier(
      {
        .image = src,
        .newLayout = ImageLayout::TransferSrc,
        .subresourceRange =
          {
            .aspectMask = aspectMask,
            .levelCount = 1,
            .layerCount = 1,
          },
      },
      {
        .stageMask = PipelineStages::Transfer,
        .accessMask = Access::TransferRead,
      })
    .imageBarrier(
      {
        .image = dst,
        .newLayout = ImageLayout::TransferDst,
        .subresourceRange =
          {
            .aspectMask = aspectMask,
            .levelCount = 1,
            .layerCount = 1,
          },
      },
      {
        .stageMask = PipelineStages::Transfer,
        .accessMask = Access::TransferWrite,
      });
  flushBarriers();

  static const auto getRegion = [](const Texture &texture) {
    const auto extent = texture.getExtent();
    return VkOffset3D{
      .x = int32_t(extent.width),
      .y = int32_t(extent.height),
      .z = 1,
    };
  };

  const VkImageBlit region{
    .srcSubresource =
      {
        .aspectMask = aspectMask,
        .mipLevel = 0,
        .layerCount = 1,
      },
    .srcOffsets = {{}, getRegion(src)},
    .dstSubresource =
      {
        .aspectMask = aspectMask,
        .mipLevel = 0,
        .layerCount = 1,
      },
    .dstOffsets = {{}, getRegion(dst)},
  };
  vkCmdBlitImage(m_handle, src.getImageHandle(),
                 VkImageLayout(src.getImageLayout()), dst.getImageHandle(),
                 VkImageLayout(dst.getImageLayout()), 1, &region, filter);
  return *this;
}

CommandBuffer &CommandBuffer::generateMipmaps(Texture &texture,
                                              const TexelFilter filter) {
  assert(texture);
  assert(_invariant(State::Recording, InvariantFlags::OutsideRenderPass));

  _TRACY_GPU_ZONE2("GenerateMipmaps");
  const auto imageHandle = texture.getImageHandle();
  m_barrierBuilder.imageBarrier(
    {
      .image = texture,
      .newLayout = ImageLayout::TransferSrc,
      .subresourceRange =
        {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .levelCount = 1,
          .layerCount = texture.m_layerFaces,
        },
    },
    {
      .stageMask = PipelineStages::Transfer,
      .accessMask = Access::TransferRead,
    });
  flushBarriers();

  // Generate the mip chain, blit level n from level n-1.
  for (auto i = 1u; i < texture.getNumMipLevels(); ++i) {
    const VkImageSubresourceRange mipSubRange{
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = i,
      .levelCount = 1,
      .layerCount = texture.m_layerFaces,
    };

    // Transition current mip level to TRANSFER_DST
    m_barrierBuilder._imageBarrier(imageHandle,
                                   {
                                     .stageMask = PipelineStages::None,
                                     .accessMask = Access::None,
                                   },
                                   {
                                     .stageMask = PipelineStages::Transfer,
                                     .accessMask = Access::TransferWrite,
                                   },
                                   ImageLayout::Undefined,
                                   ImageLayout::TransferDst, mipSubRange);
    flushBarriers();

    const VkImageBlit blitInfo{
      .srcSubresource =
        {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .mipLevel = i - 1,
          .layerCount = texture.m_layerFaces,
        },
      .srcOffsets =
        {
          {},
          {
            .x = std::max(1, int32_t(texture.m_extent.width) >> (i - 1)),
            .y = std::max(1, int32_t(texture.m_extent.height) >> (i - 1)),
            .z = std::max(1, int32_t(texture.m_depth) >> (i - 1)),
          },
        },

      .dstSubresource =
        {
          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
          .mipLevel = i,
          .layerCount = texture.m_layerFaces,
        },
      .dstOffsets =
        {
          {},
          {
            .x = std::max(1, int32_t(texture.m_extent.width) >> i),
            .y = std::max(1, int32_t(texture.m_extent.height) >> i),
            .z = std::max(1, int32_t(texture.m_depth) >> i),
          },
        },
    };
    // Blit from previous level
    vkCmdBlitImage(m_handle, imageHandle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                   imageHandle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                   &blitInfo, VkFilter(filter));

    // Transition current mip level to TRANSFER_SRC for read in next
    // iteration.
    m_barrierBuilder._imageBarrier(imageHandle,
                                   {
                                     .stageMask = PipelineStages::Blit,
                                     .accessMask = Access::TransferWrite,
                                   },
                                   {
                                     .stageMask = PipelineStages::Transfer,
                                     .accessMask = Access::TransferRead,
                                   },
                                   ImageLayout::TransferDst,
                                   ImageLayout::TransferSrc, mipSubRange);
    flushBarriers();
  }
  // After the loop, all mip layers are in TRANSFER_SRC layout.
  texture.m_layout = ImageLayout::TransferSrc;
  texture.m_lastScope = {
    .stageMask = PipelineStages::Blit,
    .accessMask = Access::TransferRead,
  };
  return *this;
}

CommandBuffer &CommandBuffer::insertFatBarrier() {
  m_barrierBuilder.memoryBarrier(kFatScope, kFatScope);
  return flushBarriers();
}
CommandBuffer &CommandBuffer::flushBarriers() {
  assert(_invariant(State::Recording, InvariantFlags::OutsideRenderPass));

  if (auto barrier = m_barrierBuilder.build(); barrier.isEffective()) {
    _TRACY_GPU_ZONE2("FlushBarriers");
    vkCmdPipelineBarrier2(m_handle, &barrier.m_info);
  }
  return *this;
}

//
// (private):
//

CommandBuffer::CommandBuffer(const VkDevice device,
                             const VkCommandPool commandPool,
                             const VkCommandBuffer handle,
                             const TracyVkCtx tracy, const VkFence fence)
    : m_device{device}, m_commandPool{commandPool}, m_state{State::Initial},
      m_handle{handle}, m_tracyContext{tracy}, m_fence{fence},
      m_descriptorSetAllocator{device} {}

bool CommandBuffer::_invariant(const State requiredState,
                               const InvariantFlags flags) const {
  if (!m_handle || m_state != requiredState) return false;

  auto valid = true;
  if (bool(flags & InvariantFlags::ValidPipeline)) {
    valid |= m_pipeline && *m_pipeline;
  }
  if (bool(flags & InvariantFlags::GraphicsPipeline)) {
    assert(m_pipeline);
    valid |= m_pipeline->getBindPoint() == VK_PIPELINE_BIND_POINT_GRAPHICS;
  }
  if (bool(flags & InvariantFlags::ComputePipeline)) {
    assert(m_pipeline);
    valid |= m_pipeline->getBindPoint() == VK_PIPELINE_BIND_POINT_COMPUTE;
  }
  if (bool(flags & InvariantFlags::InsideRenderPass)) {
    valid |= m_insideRenderPass;
  }
  if (bool(flags & InvariantFlags::OutsideRenderPass)) {
    valid |= !m_insideRenderPass;
  }
  return valid;
}

void CommandBuffer::_destroy() noexcept {
  if (m_handle != VK_NULL_HANDLE) {
    reset();

    vkDestroyFence(m_device, m_fence, nullptr);
    TracyVkDestroy(m_tracyContext);
    vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_handle);

    m_device = VK_NULL_HANDLE;
    m_commandPool = VK_NULL_HANDLE;

    m_state = State::Invalid;

    m_handle = VK_NULL_HANDLE;
    m_tracyContext = nullptr;

    m_fence = VK_NULL_HANDLE;

    m_descriptorSetAllocator = {};
    m_descriptorSetCache.clear();

    m_pipeline = nullptr;
    m_vertexBuffer = nullptr;
    m_indexBuffer = nullptr;

    m_insideRenderPass = false;
  }
}

void CommandBuffer::_chunkedUpdate(const VkBuffer bufferHandle,
                                   VkDeviceSize offset, VkDeviceSize size,
                                   const void *data) {
  const auto numChunks =
    static_cast<VkDeviceSize>(std::ceil(float(size) / float(kMaxDataSize)));
  assert(numChunks > 1);

  auto *bytes = static_cast<const std::byte *>(data);
  for (auto i = 0u; i < numChunks; ++i) {
    const auto chunkSize = std::min(size, kMaxDataSize);
    vkCmdUpdateBuffer(m_handle, bufferHandle, offset, chunkSize, bytes);

    bytes += chunkSize;
    offset += chunkSize;
    size -= chunkSize;
  }
}

void CommandBuffer::_setVertexBuffer(const VertexBuffer *vertexBuffer,
                                     const VkDeviceSize offset) {
  if (m_vertexBuffer != vertexBuffer) {
    if (vertexBuffer) {
      _TRACY_GPU_ZONE2("SetVertexBuffer");
      const auto bufferHandle = vertexBuffer->getHandle();
      vkCmdBindVertexBuffers(m_handle, 0, 1, &bufferHandle, &offset);
    }
    m_vertexBuffer = vertexBuffer;
  }
}
void CommandBuffer::_setIndexBuffer(const IndexBuffer *indexBuffer) {
  if (m_indexBuffer != indexBuffer) {
    if (indexBuffer) {
      _TRACY_GPU_ZONE2("SetIndexBuffer");
      const auto indexType = toVk(indexBuffer->getIndexType());
      vkCmdBindIndexBuffer(m_handle, indexBuffer->getHandle(), 0, indexType);
    }
    m_indexBuffer = indexBuffer;
  }
}

void CommandBuffer::_pushDebugGroup(const std::string_view label) {
  assert(_invariant(State::Recording));

  const VkDebugUtilsLabelEXT labelInfo{
    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
    .pLabelName = label.data(),
  };
  vkCmdBeginDebugUtilsLabelEXT(m_handle, &labelInfo);
}
void CommandBuffer::_popDebugGroup() {
  assert(_invariant(State::Recording));
  vkCmdEndDebugUtilsLabelEXT(m_handle);
}

//
// Utility:
//

void prepareForAttachment(CommandBuffer &cb, const Texture &texture,
                          const bool readOnly) {
  assert(texture);

  BarrierScope dst{};
  ImageLayout newLayout{ImageLayout::Undefined};

  const auto aspectMask = getAspectMask(texture);

  if (aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
    dst.stageMask = PipelineStages::ColorAttachmentOutput;
    dst.accessMask = Access::ColorAttachmentRead | Access::ColorAttachmentWrite;
    newLayout = ImageLayout::ColorAttachment;
  } else {
    dst.stageMask = PipelineStages::FragmentTests;
    dst.accessMask = readOnly ? Access::DepthStencilAttachmentRead
                              : Access::DepthStencilAttachmentWrite;

    if (aspectMask == VK_IMAGE_ASPECT_DEPTH_BIT) {
      newLayout =
        readOnly ? ImageLayout::DepthReadOnly : ImageLayout::DepthAttachment;
    } else if (aspectMask == VK_IMAGE_ASPECT_STENCIL_BIT) {
      newLayout = readOnly ? ImageLayout::StencilReadOnly
                           : ImageLayout::StencilAttachment;
    } else if (aspectMask ==
               (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
      newLayout = readOnly ? ImageLayout::DepthStencilReadOnly
                           : ImageLayout::DepthStencilAttachment;
    } else {
      assert(false);
    }
  }

  cb.getBarrierBuilder().imageBarrier(
    {
      .image = texture,
      .newLayout = newLayout,
      .subresourceRange =
        {
          .aspectMask = aspectMask,
          .levelCount = VK_REMAINING_MIP_LEVELS,
          .layerCount = VK_REMAINING_ARRAY_LAYERS,
        },
    },
    dst);
}
void prepareForReading(CommandBuffer &cb, const Texture &texture) {
  assert(texture);
  cb.getBarrierBuilder().imageBarrier(
    {
      .image = texture,
      .newLayout = ImageLayout::ShaderReadOnly,
      .subresourceRange =
        {
          .levelCount = VK_REMAINING_MIP_LEVELS,
          .layerCount = VK_REMAINING_ARRAY_LAYERS,
        },
    },
    {
      .stageMask = PipelineStages::FragmentShader,
      .accessMask = Access::ShaderRead,
    });
}

} // namespace rhi
