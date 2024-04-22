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

  auto &[cb, framebufferInfo, sets] = *static_cast<RenderContext *>(ctx);

  if (holdsAttachment(bits)) {
    if (!framebufferInfo)
      framebufferInfo.emplace().area = {.extent = texture->getExtent()};

    framebufferInfo->depthAttachment = rhi::AttachmentInfo{.target = texture};
    framebufferInfo->depthReadOnly = true;

    rhi::prepareForAttachment(cb, *texture, true);
  } else {
    const auto &[bindingInfo, type] = decodeTextureRead(bits);
    const auto [location, pipelineStage] = bindingInfo;

    assert(!bool(pipelineStage & PipelineStage::Transfer));

    auto imageLayout = rhi::ImageLayout::Undefined;

    const auto [set, binding] = location;
    switch (type) {
    case TextureRead::Type::CombinedImageSampler:
      imageLayout = rhi::ImageLayout::ReadOnly;
      sets[set][binding] =
        rhi::bindings::CombinedImageSampler{.texture = texture};
      break;
    case TextureRead::Type::SampledImage:
      imageLayout = rhi::ImageLayout::ReadOnly;
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
        .accessMask = rhi::Access::ShaderRead,
      });
  }
}

void FrameGraphTexture::preWrite(const Desc &, uint32_t bits, void *ctx) {
  ZoneScopedN("+T");

  auto &[cb, framebufferInfo, sets] = *static_cast<RenderContext *>(ctx);

  if (holdsAttachment(bits)) {
    if (!framebufferInfo)
      framebufferInfo.emplace().area = {.extent = texture->getExtent()};

    const auto attachment = decodeAttachment(bits);
    const auto aspectMask = rhi::getAspectMask(*texture);
    if (aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
      framebufferInfo->depthAttachment = makeAttachment(attachment, texture);
      framebufferInfo->depthReadOnly = false;
    } else if (aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
      auto &v = framebufferInfo->colorAttachments;
      v.resize(attachment.index + 1);
      v[attachment.index] = makeAttachment(attachment, texture);
    }

    rhi::prepareForAttachment(cb, *texture, false);
  } else {
    const auto [location, pipelineStage] = decodeBindingInfo(bits);
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
