#include "renderer/FrameGraphBuffer.hpp"
#include "StringUtility.hpp"

#include "renderer/TransientResources.hpp"

#include "FrameGraphResourceAccess.hpp"
#include "RenderContext.hpp"

// https://www.khronos.org/registry/vulkan/specs/1.3-extensions/man/html/VkAccessFlagBits.html

namespace gfx {

void FrameGraphBuffer::create(const Desc &desc, void *allocator) {
  buffer = static_cast<TransientResources *>(allocator)->acquireBuffer(desc);
}
void FrameGraphBuffer::destroy(const Desc &desc, void *allocator) {
  static_cast<TransientResources *>(allocator)->releaseBuffer(desc, buffer);
  buffer = nullptr;
}

void FrameGraphBuffer::preRead(const Desc &desc, uint32_t bits, void *ctx) {
  ZoneScoped;

  const auto [location, pipelineStage] = decodeBindingInfo(bits);
  assert(!bool(pipelineStage &
               PipelineStage::Transfer)); // GPU->CPU readback not supported.

  rhi::BarrierScope dst{};
  if (bool(pipelineStage & PipelineStage::VertexShader)) {
    switch (desc.type) {
    case BufferType::IndexBuffer:
      dst.stageMask |= rhi::PipelineStages::VertexInput;
      dst.accessMask = rhi::Access::IndexRead;
      break;
    case BufferType::VertexBuffer:
      dst.stageMask |= rhi::PipelineStages::VertexInput;
      dst.accessMask = rhi::Access::VertexAttributeRead;
      break;
    case BufferType::UniformBuffer:
      dst.stageMask |= rhi::PipelineStages::VertexShader;
      dst.accessMask = rhi::Access::UniformRead;
      break;
    case BufferType::StorageBuffer:
      dst.stageMask |= rhi::PipelineStages::VertexShader;
      dst.accessMask = rhi::Access::ShaderStorageRead;
      break;
    }
  }
  if (bool(pipelineStage & PipelineStage::GeometryShader)) {
    dst.stageMask |= rhi::PipelineStages::GeometryShader;
    dst.accessMask |= rhi::Access::ShaderRead;
  }
  if (bool(pipelineStage & PipelineStage::FragmentShader)) {
    dst.stageMask |= rhi::PipelineStages::FragmentShader;
    dst.accessMask |= rhi::Access::ShaderRead;
  }
  if (bool(pipelineStage & PipelineStage::ComputeShader)) {
    dst.stageMask |= rhi::PipelineStages::ComputeShader;
    dst.accessMask |= rhi::Access::ShaderRead;
  }

  auto &[cb, _, sets] = *static_cast<RenderContext *>(ctx);

  const auto [set, binding] = location;
  switch (desc.type) {
  case BufferType::UniformBuffer:
    sets[set][binding] = rhi::bindings::UniformBuffer{.buffer = buffer};
    break;
  case BufferType::StorageBuffer:
    sets[set][binding] = rhi::bindings::StorageBuffer{.buffer = buffer};
    break;
  }

  cb.getBarrierBuilder().bufferBarrier({.buffer = *buffer}, dst);
}
void FrameGraphBuffer::preWrite(const Desc &desc, uint32_t bits, void *ctx) {
  ZoneScoped;

  auto &[cb, _, sets] = *static_cast<RenderContext *>(ctx);
  const auto [location, pipelineStage] = decodeBindingInfo(bits);

  rhi::BarrierScope dst{};
  if (bool(pipelineStage & PipelineStage::Transfer)) {
    dst = {
      .stageMask = rhi::PipelineStages::Transfer,
      .accessMask = rhi::Access::TransferWrite,
    };
  } else {
    assert(desc.type == BufferType::StorageBuffer);

    if (bool(pipelineStage & PipelineStage::VertexShader))
      dst.stageMask |= rhi::PipelineStages::VertexShader;
    if (bool(pipelineStage & PipelineStage::GeometryShader))
      dst.stageMask |= rhi::PipelineStages::GeometryShader;
    if (bool(pipelineStage & PipelineStage::FragmentShader))
      dst.stageMask |= rhi::PipelineStages::FragmentShader;
    if (bool(pipelineStage & PipelineStage::ComputeShader))
      dst.stageMask |= rhi::PipelineStages::ComputeShader;

    dst.accessMask =
      rhi::Access::ShaderStorageRead | rhi::Access::ShaderStorageWrite;

    const auto [set, binding] = location;
    sets[set][binding] = rhi::bindings::StorageBuffer{.buffer = buffer};
  }

  cb.getBarrierBuilder().bufferBarrier({.buffer = *buffer}, dst);
}

std::string FrameGraphBuffer::toString(const Desc &desc) {
  return std::format("size: {}", formatBytes(desc.dataSize()));
}

} // namespace gfx
