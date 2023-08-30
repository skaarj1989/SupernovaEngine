#include "renderer/FrameGraphTexture.hpp"
#include "MapOptional.hpp"

#include "renderer/TransientResources.hpp"
#include "FrameGraphResourceAccess.hpp"

#include "RenderContext.hpp"

#include <format>

namespace gfx {

namespace {

template <class Target> struct StaticCast {
  template <class Source> Target operator()(Source &&source) const {
    return static_cast<Target>(std::forward<Source>(source));
  }
};

[[nodiscard]] rhi::ClearValue convert(ClearValue in) {
  switch (in) {
    using enum ClearValue;

  case Zero:
    return 0.0f;
  case One:
    return 1.0f;

  case OpaqueBlack:
    return glm::vec4{0.0f, 0.0f, 0.0f, 1.0f};
  case OpaqueWhite:
    return glm::vec4{1.0f};
  case TransparentBlack:
    return glm::vec4{0.0f};
  case TransparentWhite:
    return glm::vec4{1.0f, 1.0f, 1.0f, 0.0f};
  }

  assert(false);
  return {};
}

[[nodiscard]] auto makeAttachment(const Attachment &in, rhi::Texture *texture) {
  assert(texture && *texture);
  return rhi::AttachmentInfo{
    .target = texture,
    .layer = in.layer,
    .face = map(in.face, StaticCast<rhi::CubeFace>{}),
    .clearValue = map(in.clearValue, convert),
  };
};

} // namespace

void FrameGraphTexture::create(const Desc &desc, void *allocator) {
  texture = static_cast<TransientResources *>(allocator)->acquireTexture(desc);
}
void FrameGraphTexture::destroy(const Desc &desc, void *allocator) {
  static_cast<TransientResources *>(allocator)->releaseTexture(desc, texture);
  texture = nullptr;
}

void FrameGraphTexture::preRead(const Desc &, uint32_t bits, void *ctx) {
  ZoneScoped;

  auto &[cb, framebufferInfo, sets] = *static_cast<RenderContext *>(ctx);

  rhi::BarrierScope dst{};
  if (holdsAttachment(bits)) {
    const auto attachment = decodeAttachment(bits);
    if (!framebufferInfo)
      framebufferInfo.emplace().area = {0, 0, texture->getExtent()};

    framebufferInfo->depthAttachment = rhi::AttachmentInfo{
      .target = texture,
      .clearValue = map(attachment.clearValue, convert),
    };
    framebufferInfo->depthReadOnly = true;

    rhi::prepareForAttachment(cb, *texture, true);
  } else {
    const auto &[bindingInfo, type] = decodeTextureRead(bits);
    const auto [location, pipelineStage] = bindingInfo;

    assert(!bool(pipelineStage & PipelineStage::Transfer));

    if (bool(pipelineStage & PipelineStage::VertexShader))
      dst.stageMask |= rhi::PipelineStages::VertexShader;
    if (bool(pipelineStage & PipelineStage::GeometryShader))
      dst.stageMask |= rhi::PipelineStages::GeometryShader;
    if (bool(pipelineStage & PipelineStage::FragmentShader))
      dst.stageMask |= rhi::PipelineStages::FragmentShader;
    if (bool(pipelineStage & PipelineStage::ComputeShader))
      dst.stageMask |= rhi::PipelineStages::ComputeShader;

    dst.accessMask = rhi::Access::ShaderRead;

    auto imageLayout = rhi::ImageLayout::Undefined;

    const auto [set, binding] = location;
    switch (type) {
    case TextureRead::Type::CombinedImageSampler:
      imageLayout = rhi::ImageLayout::ShaderReadOnly;
      sets[set][binding] =
        rhi::bindings::CombinedImageSampler{.texture = texture};
      break;
    case TextureRead::Type::SampledImage:
      imageLayout = rhi::ImageLayout::ShaderReadOnly;
      sets[set][binding] = rhi::bindings::SampledImage{.texture = texture};
      break;
    case TextureRead::Type::StorageImage:
      imageLayout = rhi::ImageLayout::General;
      sets[set][binding] = rhi::bindings::StorageImage{
        .texture = texture,
        .mipLevel = 0,
      };
      break;
    }
    assert(imageLayout != rhi::ImageLayout::Undefined);

    if (imageLayout == rhi::ImageLayout::ShaderReadOnly) {
      const auto mask = rhi::getAspectMask(*texture);
      if (mask == VK_IMAGE_ASPECT_DEPTH_BIT) {
        imageLayout = rhi::ImageLayout::DepthReadOnly;
      } else if (mask == VK_IMAGE_ASPECT_STENCIL_BIT) {
        imageLayout = rhi::ImageLayout::StencilReadOnly;
      } else if (mask ==
                 (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
        imageLayout = rhi::ImageLayout::DepthStencilReadOnly;
      }
    }

    cb.getBarrierBuilder().imageBarrier(
      {
        .image = *texture,
        .newLayout = imageLayout,
        .subresourceRange =
          {
            .levelCount = VK_REMAINING_MIP_LEVELS,
            .layerCount = VK_REMAINING_ARRAY_LAYERS,
          },
      },
      dst);
  }
}

void FrameGraphTexture::preWrite(const Desc &, uint32_t bits, void *ctx) {
  ZoneScoped;

  auto &[cb, framebufferInfo, sets] = *static_cast<RenderContext *>(ctx);

  if (holdsAttachment(bits)) {
    const auto attachment = decodeAttachment(bits);
    if (!framebufferInfo)
      framebufferInfo.emplace().area = {0, 0, texture->getExtent()};

    const auto aspectMask = rhi::getAspectMask(*texture);
    if (aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
      framebufferInfo->depthAttachment = makeAttachment(attachment, texture);
      framebufferInfo->depthReadOnly = false;
    }
    if (aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
      auto &v = framebufferInfo->colorAttachments;
      v.resize(attachment.index + 1);
      v[attachment.index] = makeAttachment(attachment, texture);
    }

    rhi::prepareForAttachment(cb, *texture, false);
  } else {
    const auto [location, pipelineStage] = decodeBindingInfo(bits);

    rhi::BarrierScope dst{};
    if (bool(pipelineStage & PipelineStage::VertexShader))
      dst.stageMask |= rhi::PipelineStages::VertexShader;
    if (bool(pipelineStage & PipelineStage::GeometryShader))
      dst.stageMask |= rhi::PipelineStages::GeometryShader;
    if (bool(pipelineStage & PipelineStage::FragmentShader))
      dst.stageMask |= rhi::PipelineStages::FragmentShader;
    if (bool(pipelineStage & PipelineStage::ComputeShader))
      dst.stageMask |= rhi::PipelineStages::ComputeShader;

    dst.accessMask = rhi::Access::ShaderWrite;

    const auto [set, binding] = location;
    sets[set][binding] = rhi::bindings::StorageImage{
      .texture = texture,
      .mipLevel = 0,
    };

    cb.getBarrierBuilder().imageBarrier(
      {
        .image = *texture,
        .newLayout = rhi::ImageLayout::General,
        .subresourceRange = {.levelCount = 1, .layerCount = 1},
      },
      dst);
  }
}

std::string FrameGraphTexture::toString(const Desc &desc) {
  return std::format("{}x{} [{}]<BR/>Usage = {}", desc.extent.width,
                     desc.extent.height, rhi::toString(desc.format),
                     rhi::toString(desc.usageFlags));
}

} // namespace gfx
