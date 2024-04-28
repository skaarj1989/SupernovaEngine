#include "rhi/Texture.hpp"
#include "VisitorHelper.hpp"

#include "rhi/RenderDevice.hpp"

#include "vk_mem_alloc.h"
#include "VkCheck.hpp"

#include "glm/ext/vector_float3.hpp"
#include "glm/common.hpp"      // clamp, floor, max
#include "glm/exponential.hpp" // pow, log2

namespace rhi {

namespace {

constexpr auto kSwapchainDefaultUsageFlags =
  ImageUsage::RenderTarget | ImageUsage::TransferDst;

[[nodiscard]] auto findTextureType(const Extent2D extent, const uint32_t depth,
                                   const uint32_t numFaces,
                                   const uint32_t numLayers) {
  using enum TextureType;

  TextureType type{Undefined};
  if (numFaces == 6) {
    type = TextureCube;
  } else {
    if (depth > 0) {
      type = Texture3D;
    } else {
      type = extent.height > 0 ? Texture2D : Texture1D;
    }
  }
  if (numLayers > 0) {
    switch (type) {
    case Texture1D:
      type = Texture1DArray;
      break;
    case Texture2D:
      type = Texture2DArray;
      break;
    case TextureCube:
      type = TextureCubeArray;
      break;

    default:
      assert(false);
      type = Undefined;
    }
  }
  return type;
}

[[nodiscard]] auto getImageViewType(const TextureType textureType) {
  switch (textureType) {
    using enum TextureType;

  case Texture1D:
    return VK_IMAGE_VIEW_TYPE_1D;
  case Texture1DArray:
    return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
  case Texture2D:
    return VK_IMAGE_VIEW_TYPE_2D;
  case Texture2DArray:
    return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
  case Texture3D:
    return VK_IMAGE_VIEW_TYPE_3D;
  case TextureCube:
    return VK_IMAGE_VIEW_TYPE_CUBE;
  case TextureCubeArray:
    return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;

  default:
    assert(false);
    return VkImageViewType(~0);
  }
}

[[nodiscard]] auto isLayered(const TextureType textureType) {
  switch (textureType) {
    using enum TextureType;

  case Texture2DArray:
  case TextureCube:
  case TextureCubeArray:
    return true;

  default:
    return false;
  }
}

[[nodiscard]] auto
createImageView(const VkDevice device, const VkImage image,
                const VkImageViewType viewType, const VkFormat format,
                const VkImageSubresourceRange &subresourceRange) {
  VkImageView imageView{VK_NULL_HANDLE};
  const VkImageViewCreateInfo createInfo{
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .image = image,
    .viewType = viewType,
    .format = format,
    .subresourceRange = subresourceRange,
  };
  VK_CHECK(vkCreateImageView(device, &createInfo, nullptr, &imageView));
  return imageView;
}

[[nodiscard]] auto toVk(const ImageUsage usage,
                        const VkImageAspectFlags aspectMask) {
  VkImageUsageFlags out{0};
  if (bool(usage & ImageUsage::TransferSrc))
    out |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  if (bool(usage & ImageUsage::TransferDst))
    out |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  if (bool(usage & ImageUsage::Storage)) out |= VK_IMAGE_USAGE_STORAGE_BIT;
  if (bool(usage & ImageUsage::RenderTarget)) {
    if (aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)
      out |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    else if ((aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) ||
             (aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)) {
      out |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
  }
  if (bool(usage & ImageUsage::Sampled)) out |= VK_IMAGE_USAGE_SAMPLED_BIT;

  // UNASSIGNED-BestPractices-vkImage-DontUseStorageRenderTargets
  [[maybe_unused]] constexpr auto kForbiddenSet =
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
  assert((out & kForbiddenSet) != kForbiddenSet);
  return out;
}

} // namespace

//
// Texture class:
//

// clang-format off
Texture::Texture(Texture &&other) noexcept
    : m_deviceOrAllocator{std::move(other.m_deviceOrAllocator)},
      m_image{std::move(other.m_image)},
  
      m_type{other.m_type},

      m_layout(other.m_layout),
      m_lastScope{std::move(other.m_lastScope)},
      
      m_aspects{std::move(other.m_aspects)}, 
      m_sampler{other.m_sampler},
  
      m_extent{other.m_extent},
      m_depth{other.m_depth},
      m_format{other.m_format},
      m_numMipLevels{other.m_numMipLevels},
      m_numLayers{other.m_numLayers},
      m_layerFaces{other.m_layerFaces}, 
      m_usageFlags{other.m_usageFlags} {
  other.m_deviceOrAllocator = {};
  other.m_image = {};

  other.m_type = TextureType::Undefined;
  other.m_layout = ImageLayout::Undefined;

  other.m_sampler = VK_NULL_HANDLE;

  other.m_format = PixelFormat::Undefined;
}
// clang-format on
Texture::~Texture() { _destroy(); }

Texture &Texture::operator=(Texture &&rhs) noexcept {
  if (this != &rhs) {
    _destroy();

    std::swap(m_deviceOrAllocator, rhs.m_deviceOrAllocator);
    std::swap(m_image, rhs.m_image);

    std::swap(m_type, rhs.m_type);

    std::swap(m_layout, rhs.m_layout);
    std::swap(m_lastScope, rhs.m_lastScope);

    std::swap(m_aspects, rhs.m_aspects);
    std::swap(m_sampler, rhs.m_sampler);

    std::swap(m_extent, rhs.m_extent);
    std::swap(m_depth, rhs.m_depth);
    std::swap(m_format, rhs.m_format);
    std::swap(m_numMipLevels, rhs.m_numMipLevels);
    std::swap(m_numLayers, rhs.m_numLayers);
    std::swap(m_layerFaces, rhs.m_layerFaces);
    std::swap(m_usageFlags, rhs.m_usageFlags);
  }
  return *this;
}

bool Texture::operator==(const Texture &other) const {
  return m_image == other.m_image;
}
Texture::operator bool() const {
  return !m_image.valueless_by_exception() &&
         !std::holds_alternative<std::monostate>(m_image);
}

void Texture::setSampler(const VkSampler sampler) { m_sampler = sampler; }

TextureType Texture::getType() const { return m_type; }
Extent2D Texture::getExtent() const { return m_extent; }
uint32_t Texture::getDepth() const { return m_depth; }
uint32_t Texture::getNumMipLevels() const { return m_numMipLevels; }
uint32_t Texture::getNumLayers() const { return m_numLayers; }
PixelFormat Texture::getPixelFormat() const { return m_format; }
ImageUsage Texture::getUsageFlags() const { return m_usageFlags; }

VkImage Texture::getImageHandle() const {
  return std::visit(
    Overload{
      [](const std::monostate) -> VkImage { return VK_NULL_HANDLE; },
      [](const VkImage image) { return image; },
      [](const AllocatedImage &allocatedImage) {
        return allocatedImage.handle;
      },
    },
    m_image);
}
ImageLayout Texture::getImageLayout() const { return m_layout; }

VkImageView Texture::getImageView(const VkImageAspectFlags aspectMask) const {
  const auto *aspect = _getAspect(aspectMask);
  return aspect ? aspect->imageView : VK_NULL_HANDLE;
}

VkImageView Texture::getMipLevel(const uint32_t index,
                                 const VkImageAspectFlags aspectMask) const {
  const auto safeIndex = glm::clamp(index, 0u, m_numMipLevels - 1);
  assert(index == safeIndex);
  const auto *aspect = _getAspect(aspectMask);
  return aspect ? aspect->mipLevels[safeIndex] : VK_NULL_HANDLE;
}
std::span<const VkImageView>
Texture::getMipLevels(const VkImageAspectFlags aspectMask) const {
  const auto *aspect = _getAspect(aspectMask);
  return aspect ? aspect->mipLevels : std::span<const VkImageView>{};
}
VkImageView Texture::getLayer(const uint32_t layer,
                              const std::optional<CubeFace> face,
                              const VkImageAspectFlags aspectMask) const {
  const auto i = face ? (layer * 6) + uint32_t(*face) : layer;
  const auto safeIndex = glm::clamp(i, 0u, m_layerFaces - 1);
  assert(i == safeIndex);
  const auto *aspect = _getAspect(aspectMask);
  return aspect ? aspect->layers[safeIndex] : VK_NULL_HANDLE;
}
std::span<const VkImageView>
Texture::getLayers(const VkImageAspectFlags aspectMask) const {
  const auto *aspect = _getAspect(aspectMask);
  return aspect ? aspect->layers : std::span<const VkImageView>{};
}

VkSampler Texture::getSampler() const { return m_sampler; }

//
// (private):
//

Texture::Texture(const VmaAllocator memoryAllocator, CreateInfo &&ci)
    : m_deviceOrAllocator{memoryAllocator} {
  assert(ci.extent &&
         (ci.numFaces != 6 || ci.extent.width == ci.extent.height));

  m_type = findTextureType(ci.extent, ci.depth, ci.numFaces, ci.numLayers);
  assert(m_type != TextureType::Undefined);

  VkImageCreateFlags flags{0u};
  if (ci.numFaces == 6) flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  if (bool(ci.usageFlags & ImageUsage::RenderTarget) &&
      m_type == TextureType::Texture3D) {
    flags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT;
  }

  if (ci.numMipLevels == 0) {
    ci.numMipLevels =
      calcMipLevels(glm::max(ci.extent.width, ci.extent.height));
  }
  const auto layerFaces = ci.numFaces * std::max(1u, ci.numLayers);
  const auto aspectMask = getAspectMask(ci.pixelFormat);

  const VkImageCreateInfo imageInfo{
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .flags = flags,
    .imageType =
      m_type == TextureType::Texture3D ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D,
    .format = VkFormat(ci.pixelFormat),
    .extent =
      {
        .width = ci.extent.width,
        .height = ci.extent.height,
        .depth = std::max(1u, ci.depth),
      },
    .mipLevels = ci.numMipLevels,
    .arrayLayers = layerFaces,
    .samples =
      VK_SAMPLE_COUNT_1_BIT, // Don't care about multisampling right now.
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = toVk(ci.usageFlags, aspectMask),
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    // UNASSIGNED-BestPractices-TransitionUndefinedToReadOnly
    .initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED,
  };
  const VmaAllocationCreateInfo allocationCreateInfo{
    .usage = VMA_MEMORY_USAGE_GPU_ONLY,
  };
  AllocatedImage image;
  VK_CHECK(vmaCreateImage(memoryAllocator, &imageInfo, &allocationCreateInfo,
                          &image.handle, &image.allocation, nullptr));
  m_image = image;

  m_layout = ImageLayout(imageInfo.initialLayout);

  m_extent = ci.extent;
  m_depth = ci.depth;
  m_format = ci.pixelFormat;
  m_numMipLevels = ci.numMipLevels;
  m_numLayers = ci.numLayers;
  m_layerFaces = layerFaces;
  m_usageFlags = ci.usageFlags;

  VmaAllocatorInfo allocatorInfo{};
  vmaGetAllocatorInfo(memoryAllocator, &allocatorInfo);

  const auto imageViewType = getImageViewType(m_type);

  const auto device = _getDeviceHandle();
  _createAspect(device, image.handle, imageViewType, aspectMask,
                m_aspects[aspectMask]);
  if (aspectMask == (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) {
    _createAspect(device, image.handle, imageViewType,
                  VK_IMAGE_ASPECT_DEPTH_BIT,
                  m_aspects[VK_IMAGE_ASPECT_DEPTH_BIT]);
    _createAspect(device, image.handle, imageViewType,
                  VK_IMAGE_ASPECT_STENCIL_BIT,
                  m_aspects[VK_IMAGE_ASPECT_STENCIL_BIT]);
  }
}
Texture::Texture(const VkDevice device, const VkImage handle,
                 const Extent2D extent, const PixelFormat pixelFormat)
    : m_deviceOrAllocator{device}, m_image{handle},
      m_type{TextureType::Texture2D}, m_extent{extent}, m_format{pixelFormat},
      m_usageFlags{kSwapchainDefaultUsageFlags} {
  m_aspects[VK_IMAGE_ASPECT_COLOR_BIT].imageView = createImageView(
    device, handle, VK_IMAGE_VIEW_TYPE_2D, VkFormat(pixelFormat),
    {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .baseMipLevel = 0,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .layerCount = 1,
    });
}

void Texture::_destroy() noexcept {
  if (!bool(*this)) return;

  m_sampler = VK_NULL_HANDLE;

  const auto device = _getDeviceHandle();
  assert(device != VK_NULL_HANDLE);

  for (auto &[_, data] : m_aspects) {
    for (const auto layer : data.layers) {
      vkDestroyImageView(device, layer, nullptr);
    }
    data.layers.clear();
    for (const auto mipLevel : data.mipLevels) {
      vkDestroyImageView(device, mipLevel, nullptr);
    }
    data.mipLevels.clear();

    if (data.imageView != VK_NULL_HANDLE) {
      vkDestroyImageView(device, data.imageView, nullptr);
      data.imageView = VK_NULL_HANDLE;
    }
  }

  if (const auto allocatedImage = std::get_if<AllocatedImage>(&m_image);
      allocatedImage) {
    vmaDestroyImage(std::get<VmaAllocator>(m_deviceOrAllocator),
                    allocatedImage->handle, allocatedImage->allocation);
  }
  m_deviceOrAllocator = {};
  m_image = {};

  m_type = TextureType::Undefined;

  m_layout = ImageLayout::Undefined;

  m_extent = {};
  m_depth = 0u;
  m_format = PixelFormat::Undefined;
  m_numMipLevels = 0u;
  m_numLayers = 0u;
  m_layerFaces = 0u;
}

VkDevice Texture::_getDeviceHandle() const {
  return std::visit(
    Overload{
      [](const std::monostate) -> VkDevice { return VK_NULL_HANDLE; },
      [](const VkDevice device) { return device; },
      [](const VmaAllocator allocator) {
        VmaAllocatorInfo allocatorInfo;
        vmaGetAllocatorInfo(allocator, &allocatorInfo);
        return allocatorInfo.device;
      },
    },
    m_deviceOrAllocator);
}

void Texture::_createAspect(const VkDevice device, const VkImage image,
                            const VkImageViewType viewType,
                            const VkImageAspectFlags aspectMask,
                            AspectData &data) {
  const auto format = static_cast<VkFormat>(m_format);
  data.imageView = createImageView(device, image, viewType, format,
                                   {
                                     .aspectMask = aspectMask,
                                     .levelCount = m_numMipLevels,
                                     .layerCount = m_layerFaces,
                                   });

  data.mipLevels.reserve(m_numMipLevels);
  for (auto i = 0u; i < m_numMipLevels; ++i) {
    data.mipLevels.emplace_back(createImageView(device, image, viewType, format,
                                                {
                                                  .aspectMask = aspectMask,
                                                  .baseMipLevel = i,
                                                  .levelCount = 1u,
                                                  .layerCount = m_layerFaces,
                                                }));
  }

  if (isLayered(m_type)) {
    data.layers.reserve(m_layerFaces);
    for (auto i = 0u; i < m_layerFaces; ++i) {
      data.layers.emplace_back(createImageView(device, image,
                                               VK_IMAGE_VIEW_TYPE_2D, format,
                                               {
                                                 .aspectMask = aspectMask,
                                                 .levelCount = 1u,
                                                 .baseArrayLayer = i,
                                                 .layerCount = 1u,
                                               }));
    }
  }
}
const Texture::AspectData *
Texture::_getAspect(const VkImageAspectFlags aspectMask) const {
  auto it = m_aspects.find(
    aspectMask == VK_IMAGE_ASPECT_NONE ? getAspectMask(m_format) : aspectMask);
  return it != m_aspects.end() ? &it->second : nullptr;
}

//
// Builder:
//

using Builder = Texture::Builder;

Builder &Builder::setExtent(const Extent2D extent, const uint32_t depth) {
  m_extent = extent;
  m_depth = depth;
  return *this;
}
Builder &Builder::setPixelFormat(const PixelFormat pixelFormat) {
  m_pixelFormat = pixelFormat;
  return *this;
}
Builder &Builder::setNumMipLevels(const std::optional<uint32_t> i) {
  assert(!i || *i > 0);
  m_numMipLevels = i;
  return *this;
}
Builder &Builder::setNumLayers(const std::optional<uint32_t> i) {
  assert(!i || *i > 0);
  m_numLayers = i;
  return *this;
}
Builder &Builder::setCubemap(const bool b) {
  m_isCubemap = b;
  return *this;
}
Builder &Builder::setUsageFlags(const ImageUsage flags) {
  m_usageFlags = flags;
  return *this;
}
Builder &Builder::setupOptimalSampler(const bool enabled) {
  m_setupOptimalSampler = enabled;
  return *this;
}

Texture Builder::build(RenderDevice &rd) {
  if (!isFormatSupported(rd, m_pixelFormat, m_usageFlags)) {
    return {};
  }

  ZoneScopedN("RHI::BuildTexture");

  Texture texture{};
  if (m_isCubemap) {
    texture = rd.createCubemap(m_extent.width, m_pixelFormat,
                               m_numMipLevels.value_or(0),
                               m_numLayers.value_or(0), m_usageFlags);
  } else if (m_depth > 0) {
    texture = rd.createTexture3D(m_extent, m_depth, m_pixelFormat,
                                 m_numMipLevels.value_or(0), m_usageFlags);
  } else {
    texture =
      rd.createTexture2D(m_extent, m_pixelFormat, m_numMipLevels.value_or(0),
                         m_numLayers.value_or(0), m_usageFlags);
  }
  assert(texture);

  if (m_setupOptimalSampler) {
    const auto numMipLevels = texture.getNumMipLevels();
    rd.setupSampler(texture,
                    {
                      .magFilter = TexelFilter::Linear,
                      .minFilter = TexelFilter::Linear,
                      .mipmapMode = numMipLevels > 1 ? MipmapMode::Linear
                                                     : MipmapMode::Nearest,
                      .maxAnisotropy = 16.0f,
                      .maxLod = float(numMipLevels),
                    });
  }

  return texture;
}

//
// Utility:
//

bool isFormatSupported(const RenderDevice &rd, const PixelFormat pixelFormat,
                       const ImageUsage usageFlags) {
  VkFormatFeatureFlags requiredFeatureFlags{0};
  if (bool(usageFlags & ImageUsage::TransferSrc)) {
    requiredFeatureFlags |= VK_FORMAT_FEATURE_TRANSFER_SRC_BIT;
  }
  if (bool(usageFlags & ImageUsage::TransferDst)) {
    requiredFeatureFlags |= VK_FORMAT_FEATURE_TRANSFER_DST_BIT;
  }
  if (bool(usageFlags & ImageUsage::Storage)) {
    requiredFeatureFlags |= VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT;
  }
  if (bool(usageFlags & ImageUsage::RenderTarget)) {
    const auto aspectMask = getAspectMask(pixelFormat);
    if (aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) {
      requiredFeatureFlags |= VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT;
    }
    if (aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT) {
      requiredFeatureFlags |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT) {
      requiredFeatureFlags |= VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
  }
  if (bool(usageFlags & ImageUsage::Sampled)) {
    requiredFeatureFlags |= VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT;
  }

  const auto formatProperties = rd.getFormatProperties(pixelFormat);
  return (formatProperties.optimalTilingFeatures & requiredFeatureFlags) ==
         requiredFeatureFlags;
}

VkImageAspectFlags getAspectMask(const Texture &texture) {
  return getAspectMask(texture.getPixelFormat());
}

uint32_t calcMipLevels(const Extent2D extent) {
  return calcMipLevels(glm::max(extent.width, extent.height));
}
uint32_t calcMipLevels(const uint32_t size) {
  return uint32_t(glm::floor(glm::log2(float(size)))) + 1u;
}
glm::uvec3 calcMipSize(const glm::uvec3 &baseSize, const uint32_t level) {
  return glm::vec3{baseSize} * glm::pow(0.5f, float(level));
}

bool isCubemap(const Texture &texture) {
  assert(texture);

  switch (texture.getType()) {
    using enum TextureType;

  case TextureCube:
  case TextureCubeArray:
    return true;

  default:
    return false;
  }
}

} // namespace rhi
