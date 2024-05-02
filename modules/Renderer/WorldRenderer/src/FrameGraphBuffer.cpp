#include "renderer/FrameGraphBuffer.hpp"
#include "rhi/CommandBuffer.hpp"
#include "StringUtility.hpp"

#include "renderer/TransientResources.hpp"
#include "FrameGraphResourceAccess.hpp"
#include "RenderContext.hpp"

#include "tracy/Tracy.hpp"

#include <format>

// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPipelineStageFlagBits2.html#_description
// https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkAccessFlagBits2.html#_description

#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wswitch"
#endif

namespace gfx {

void FrameGraphBuffer::create(const Desc &desc, void *allocator) {
  buffer = static_cast<TransientResources *>(allocator)->acquireBuffer(desc);
}
void FrameGraphBuffer::destroy(const Desc &desc, void *allocator) {
  static_cast<TransientResources *>(allocator)->releaseBuffer(desc, buffer);
  buffer = nullptr;
}

void FrameGraphBuffer::preRead(const Desc &desc, uint32_t bits, void *ctx) {
  ZoneScopedN("B*");

  auto &rc = *static_cast<RenderContext *>(ctx);
  rhi::BarrierScope dst{};

  const auto bindingInfo = decodeBindingInfo(bits);
  if (bool(bindingInfo.pipelineStage & PipelineStage::Transfer)) {
    dst.accessMask = rhi::Access::TransferRead;
  } else {
    switch (desc.type) {
    case BufferType::IndexBuffer:
      dst = {
        .stageMask = rhi::PipelineStages::VertexInput,
        .accessMask = rhi::Access::IndexRead,
      };
      break;
    case BufferType::VertexBuffer:
      dst = {
        .stageMask = rhi::PipelineStages::VertexInput,
        .accessMask = rhi::Access::VertexAttributeRead,
      };
      break;
    case BufferType::UniformBuffer:
      dst.accessMask = rhi::Access::UniformRead;
      break;
    case BufferType::StorageBuffer:
      dst.accessMask = rhi::Access::ShaderStorageRead;
      break;
    }

    const auto [set, binding] = bindingInfo.location;
    switch (desc.type) {
    case BufferType::UniformBuffer:
      rc.resourceSet[set][binding] =
        rhi::bindings::UniformBuffer{.buffer = buffer};
      break;
    case BufferType::StorageBuffer:
      rc.resourceSet[set][binding] =
        rhi::bindings::StorageBuffer{.buffer = buffer};
      break;
    }
  }
  dst.stageMask |= convert(bindingInfo.pipelineStage);
  rc.commandBuffer.getBarrierBuilder().bufferBarrier({.buffer = *buffer}, dst);
}
void FrameGraphBuffer::preWrite([[maybe_unused]] const Desc &desc,
                                uint32_t bits, void *ctx) {
  ZoneScopedN("+B");

  auto &rc = *static_cast<RenderContext *>(ctx);
  const auto [location, pipelineStage] = decodeBindingInfo(bits);

  rhi::BarrierScope dst{};
  if (bool(pipelineStage & PipelineStage::Transfer)) {
    dst = {
      .stageMask = rhi::PipelineStages::Transfer,
      .accessMask = rhi::Access::TransferWrite,
    };
  } else {
    assert(desc.type == BufferType::StorageBuffer);
    dst.stageMask |= convert(pipelineStage);
    dst.accessMask =
      rhi::Access::ShaderStorageRead | rhi::Access::ShaderStorageWrite;

    const auto [set, binding] = location;
    rc.resourceSet[set][binding] =
      rhi::bindings::StorageBuffer{.buffer = buffer};
  }
  rc.commandBuffer.getBarrierBuilder().bufferBarrier({.buffer = *buffer}, dst);
}

std::string FrameGraphBuffer::toString(const Desc &desc) {
  return std::format("size: {}", formatBytes(desc.dataSize()));
}

} // namespace gfx
