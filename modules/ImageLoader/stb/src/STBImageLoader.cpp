#include "STBImageLoader.hpp"
#include "os/FileSystem.hpp"
#include "rhi/TextureUtility.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::expected<rhi::Texture, std::string>
loadTextureSTB(const std::filesystem::path &p, rhi::RenderDevice &rd) {
  stbi_set_flip_vertically_on_load(false);

  auto file = stbi__fopen(p.string().c_str(), "rb");
  if (!file) {
    return std::unexpected{"Could not open the file."};
  }

  const auto hdr = stbi_is_hdr_from_file(file);

  int32_t width;
  int32_t height;

  struct Deleter {
    void operator()(void *pixels) const { stbi_image_free(pixels); }
  };
  std::unique_ptr<void, Deleter> pixels;
  {
    auto ptr = hdr ? static_cast<void *>(stbi_loadf_from_file(
                       file, &width, &height, nullptr, STBI_rgb_alpha))
                   : static_cast<void *>(stbi_load_from_file(
                       file, &width, &height, nullptr, STBI_rgb_alpha));
    pixels.reset(ptr);
  }
  fclose(file);

  if (!pixels) {
    return std::unexpected{stbi_failure_reason()};
  }

  const auto extent = rhi::Extent2D{uint32_t(width), uint32_t(height)};
  const auto numMipLevels = rhi::calcMipLevels(extent);

  const auto generateMipmaps = numMipLevels > 1;

  auto usageFlags = rhi::ImageUsage::TransferDst | rhi::ImageUsage::Sampled;
  if (generateMipmaps) usageFlags |= rhi::ImageUsage::TransferSrc;

  const auto pixelFormat =
    hdr ? rhi::PixelFormat::RGBA32F : rhi::PixelFormat::RGBA8_UNorm;
  auto texture = rhi::Texture::Builder{}
                   .setExtent(extent)
                   .setPixelFormat(pixelFormat)
                   .setNumMipLevels(numMipLevels)
                   .setNumLayers(std::nullopt)
                   .setUsageFlags(usageFlags)
                   .setupOptimalSampler(true)
                   .build(rd);
  if (!texture) {
    return std::unexpected{
      std::format("Unsupported pixel format: VkFormat({}).",
                  std::to_underlying(pixelFormat))};
  }

  const auto pixelSize = int32_t(hdr ? sizeof(float) : sizeof(uint8_t));
  const auto uploadSize = width * height * STBI_rgb_alpha * pixelSize;
  auto srcStagingBuffer = rd.createStagingBuffer(uploadSize, pixels.get());
  rhi::upload(rd, srcStagingBuffer, {}, texture, generateMipmaps);

  return texture;
}
