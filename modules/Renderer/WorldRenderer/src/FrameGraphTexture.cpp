#include "renderer/FrameGraphTexture.hpp"
#include "MapOptional.hpp"
#include "rhi/CommandBuffer.hpp"

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

[[nodiscard]] rhi::ClearValue convert(const ClearValue in) {
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

  case UIntMax:
    return glm::uvec4{UINT_MAX};
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
}

} // namespace

//
// FrameGraphTexture class:
//

void FrameGraphTexture::create(const Desc &desc, void *allocator) {
  texture = static_cast<TransientResources *>(allocator)->acquireTexture(desc);
}
void FrameGraphTexture::destroy(const Desc &desc, void *allocator) {
  static_cast<TransientResources *>(allocator)->releaseTexture(desc, texture);
  texture = nullptr;
}

void FrameGraphTexture::preRead(const Desc &, uint32_t bits, void *ctx) {
  ZoneScopedN("T*");

  auto &[cb, _, framebufferInfo, sets] = *static_cast<RenderContext *>(ctx);

  if (holdsAttachment(bits)) {
    if (!framebufferInfo)
      framebufferInfo.emplace().area = {.extent = texture->getExtent()};

    switch (decodeAttachment(bits).imageAspect) {
      using enum rhi::ImageAspect;

    case Depth:
      framebufferInfo->depthAttachment = rhi::AttachmentInfo{.target = texture};
      framebufferInfo->depthReadOnly = true;
      break;
    case Stencil:
      framebufferInfo->stencilAttachment =
        rhi::AttachmentInfo{.target = texture};
      framebufferInfo->stencilReadOnly = true;
      break;

    default:
      assert(false);
    }

    rhi::prepareForAttachment(cb, *texture, true);
  } else {
    const auto &[bindingInfo, type, imageAspect] = decodeTextureRead(bits);
    const auto [location, pipelineStage] = bindingInfo;

    auto imageLayout = rhi::ImageLayout::Undefined;
    auto dstAccess = rhi::Access::None;

    if (bool(pipelineStage & PipelineStage::Transfer)) {
      imageLayout = rhi::ImageLayout::TransferSrc;
      dstAccess = rhi::Access::TransferRead;
    } else {
      const auto [set, binding] = location;
      switch (type) {
        using enum TextureRead::Type;

      case CombinedImageSampler:
        imageLayout = rhi::ImageLayout::ReadOnly;
        sets[set][binding] = rhi::bindings::CombinedImageSampler{
          .texture = texture,
          .imageAspect = imageAspect,
        };
        break;
      case SampledImage:
        imageLayout = rhi::ImageLayout::ReadOnly;
        sets[set][binding] = rhi::bindings::SampledImage{
          .texture = texture,
          .imageAspect = imageAspect,
        };
        break;
      case StorageImage:
        imageLayout = rhi::ImageLayout::General;
        sets[set][binding] = rhi::bindings::StorageImage{
          .texture = texture,
          .imageAspect = imageAspect,
          .mipLevel = 0,
        };
        break;
      }
      dstAccess = rhi::Access::ShaderRead;
    }

    assert(imageLayout != rhi::ImageLayout::Undefined);

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
      {
        .stageMask = convert(pipelineStage),
        .accessMask = dstAccess,
      });
  }
}
void FrameGraphTexture::preWrite(const Desc &, uint32_t bits, void *ctx) {
  ZoneScopedN("+T");

  auto &[cb, _, framebufferInfo, sets] = *static_cast<RenderContext *>(ctx);

  if (holdsAttachment(bits)) {
    if (!framebufferInfo)
      framebufferInfo.emplace().area = {.extent = texture->getExtent()};

    const auto attachment = decodeAttachment(bits);

    switch (attachment.imageAspect) {
      using enum rhi::ImageAspect;

    case Depth:
      framebufferInfo->depthAttachment = makeAttachment(attachment, texture);
      framebufferInfo->depthReadOnly = false;
      break;
    case Stencil:
      framebufferInfo->stencilAttachment = makeAttachment(attachment, texture);
      framebufferInfo->stencilReadOnly = false;
      break;
    case Color: {
      auto &v = framebufferInfo->colorAttachments;
      v.resize(attachment.index + 1);
      v[attachment.index] = makeAttachment(attachment, texture);
    } break;
    }

    rhi::prepareForAttachment(cb, *texture, false);
  } else {
    const auto [bindingInfo, imageAspect] = decodeImageWrite(bits);
    assert(imageAspect != rhi::ImageAspect::None);
    const auto [location, pipelineStage] = bindingInfo;
    const auto [set, binding] = location;
    sets[set][binding] = rhi::bindings::StorageImage{
      .texture = texture,
      .imageAspect = imageAspect,
      .mipLevel = 0,
    };

    cb.getBarrierBuilder().imageBarrier(
      {
        .image = *texture,
        .newLayout = rhi::ImageLayout::General,
        .subresourceRange = {.levelCount = 1, .layerCount = 1},
      },
      {
        .stageMask = convert(pipelineStage),
        .accessMask = rhi::Access::ShaderStorageWrite,
      });
  }
}

std::string FrameGraphTexture::toString(const Desc &desc) {
  return std::format("{}x{} [{}]<BR/>Usage = {}", desc.extent.width,
                     desc.extent.height, rhi::toString(desc.format),
                     rhi::toString(desc.usageFlags));
}

} // namespace gfx
