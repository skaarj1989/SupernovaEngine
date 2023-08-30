#include "KTXLoader.hpp"
#include "rhi/TextureUtility.hpp"
#include "os/FileSystem.hpp"

#include "ktxvulkan.h" // ktxTexture_GetVkFormat

#include <numeric> // lcm

// https://github.com/KhronosGroup/KTX-Software/blob/master/lib/vkloader.c

namespace {

struct CallbackDataOptimal {
  VkBufferImageCopy *region;
  VkDeviceSize offset;

  ktx_uint32_t numFaces;
  ktx_uint32_t numLayers;

  // The following are used only by optimalTilingPadCallback:

  ktx_uint8_t *dest; // Pointer to mapped staging buffer.
  ktx_uint32_t elementSize;
  ktx_uint32_t numDimensions;
};

[[nodiscard]] KTX_error_code
optimalTilingCallback(int miplevel, int face, int width, int height, int depth,
                      ktx_uint64_t faceLodSize, void *, void *userdata) {
  auto *ud = static_cast<CallbackDataOptimal *>(userdata);

  *ud->region = {
    .bufferOffset = ud->offset,

    .bufferRowLength = 0,
    .bufferImageHeight = 0,

    .imageSubresource =
      {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = uint32_t(miplevel),
        .baseArrayLayer = uint32_t(face),
        .layerCount = ud->numLayers * ud->numFaces,
      },

    .imageOffset = {.x = 0, .y = 0, .z = 0},
    .imageExtent =
      {
        .width = uint32_t(width),
        .height = uint32_t(height),
        .depth = uint32_t(depth),
      },
  };

  ud->offset += faceLodSize;
  ud->region++;

  return KTX_SUCCESS;
}

// Taken from: ktxint.h

#define _KTX_PADN(n, nbytes) (ktx_uint32_t)(n * ceilf((float)(nbytes) / n))
#define _KTX_PADN_LEN(n, nbytes)                                               \
  (ktx_uint32_t)((n * ceilf((float)(nbytes) / n)) - (nbytes))
#define _KTX_PAD_UNPACK_ALIGN(nbytes) _KTX_PADN(KTX_GL_UNPACK_ALIGNMENT, nbytes)
#define _KTX_PAD_UNPACK_ALIGN_LEN(nbytes)                                      \
  _KTX_PADN_LEN(KTX_GL_UNPACK_ALIGNMENT, nbytes)

KTX_error_code optimalTilingPadCallback(int miplevel, int face, int width,
                                        int height, int depth,
                                        ktx_uint64_t faceLodSize, void *pixels,
                                        void *userdata) {
  auto *ud = static_cast<CallbackDataOptimal *>(userdata);

  ud->region->bufferOffset = ud->offset;

  auto rowPitch = width * ud->elementSize;
  if (_KTX_PAD_UNPACK_ALIGN_LEN(rowPitch) == 0) {
    std::memcpy(ud->dest + ud->offset, pixels, faceLodSize);
    ud->offset += faceLodSize;
  } else {

    ktx_uint32_t imageIterations;
    if (ud->numDimensions == 3)
      imageIterations = depth;
    else if (ud->numLayers > 1)
      imageIterations = ud->numLayers * ud->numFaces;
    else
      imageIterations = 1;

    rowPitch = width * ud->elementSize;
    const auto paddedRowPitch = _KTX_PAD_UNPACK_ALIGN(rowPitch);
    for (auto image = 0; image < imageIterations; ++image) {
      for (auto row = 0; row < height; ++row) {
        std::memcpy(ud->dest + ud->offset, pixels, rowPitch);
        ud->offset += rowPitch;
        pixels = static_cast<ktx_uint8_t *>(pixels) + paddedRowPitch;
      }
    }
  }

  if (ud->offset % ud->elementSize != 0 || ud->offset % 4 != 0) {
    assert(ud->elementSize < 4 && ud->elementSize > 0);
    const auto lcm = std::lcm(ud->elementSize, 4u);
    ud->offset = _KTX_PADN(lcm, ud->offset);
  }

  ud->region->bufferRowLength = 0;
  ud->region->bufferImageHeight = 0;

  ud->region->imageSubresource = {
    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    .mipLevel = uint32_t(miplevel),
    .baseArrayLayer = uint32_t(face),
    .layerCount = ud->numLayers * ud->numFaces,
  };

  ud->region->imageOffset = {.x = 0, .y = 0, .z = 0};
  ud->region->imageExtent = {
    .width = uint32_t(width),
    .height = uint32_t(height),
    .depth = uint32_t(depth),
  };

  ud->region++;

  return KTX_SUCCESS;
}

[[nodiscard]] const char *toString(ktx_error_code_e err) {
#define CASE(Value)                                                            \
  case Value:                                                                  \
    return #Value

  switch (err) {
    CASE(KTX_SUCCESS);
    CASE(KTX_FILE_DATA_ERROR);
    CASE(KTX_FILE_ISPIPE);
    CASE(KTX_FILE_OPEN_FAILED);
    CASE(KTX_FILE_OVERFLOW);
    CASE(KTX_FILE_READ_ERROR);
    CASE(KTX_FILE_SEEK_ERROR);
    CASE(KTX_FILE_UNEXPECTED_EOF);
    CASE(KTX_FILE_WRITE_ERROR);
    CASE(KTX_GL_ERROR);
    CASE(KTX_INVALID_OPERATION);
    CASE(KTX_INVALID_VALUE);
    CASE(KTX_NOT_FOUND);
    CASE(KTX_OUT_OF_MEMORY);
    CASE(KTX_TRANSCODE_FAILED);
    CASE(KTX_UNKNOWN_FILE_FORMAT);
    CASE(KTX_UNSUPPORTED_TEXTURE_TYPE);
    CASE(KTX_UNSUPPORTED_FEATURE);
    CASE(KTX_LIBRARY_NOT_LINKED);
  }
#undef CASE

  assert(false);
  return "UNKNOWN";
}

[[nodiscard]] auto upload(rhi::RenderDevice &rd, rhi::Texture &texture,
                          ktxTexture *ktx) {
  const auto elementSize = ktxTexture_GetElementSize(ktx);
  ktx_bool_t canUseFasterPath{VK_TRUE};
  if (ktx->classId == ktxTexture2_c) {
    canUseFasterPath = KTX_TRUE;
  } else {
    const auto actualRowPitch = ktxTexture_GetRowPitch(ktx, 0);
    const auto tightRowPitch = elementSize * ktx->baseWidth;
    if (elementSize % 4 == 0 ||
        (ktx->numLevels == 1 && actualRowPitch == tightRowPitch)) {
      canUseFasterPath = KTX_TRUE;
    } else {
      canUseFasterPath = KTX_FALSE;
    }
  }

  auto dataSize = ktxTexture_GetDataSizeUncompressed(ktx);
  ktx_uint32_t numCopyRegions;
  if (canUseFasterPath) {
    numCopyRegions = ktx->numLevels;
  } else {
    numCopyRegions =
      ktx->isArray ? ktx->numLevels : ktx->numLevels * ktx->numFaces;
    dataSize += numCopyRegions * elementSize * 4;
  }

  auto srcStagingBuffer = rd.createStagingBuffer(dataSize);
  auto *pMappedStagingBuffer =
    static_cast<ktx_uint8_t *>(srcStagingBuffer.map());

  std::vector<VkBufferImageCopy> copyRegions;
  copyRegions.resize(numCopyRegions);

  CallbackDataOptimal cbData{
    .region = copyRegions.data(),
    .offset = 0,
    .numFaces = ktx->numFaces,
    .numLayers = ktx->numLayers,
    .dest = pMappedStagingBuffer,
    .elementSize = elementSize,
    .numDimensions = ktx->numDimensions,
  };

  KTX_error_code result;
  if (canUseFasterPath) {
    if (ktx->pData) {
      assert(ktx->dataSize <= srcStagingBuffer.getSize());
      std::memcpy(pMappedStagingBuffer, ktx->pData, ktx->dataSize);
    } else {
      result = ktxTexture_LoadImageData(ktx, pMappedStagingBuffer,
                                        srcStagingBuffer.getSize());
      if (result != KTX_SUCCESS) return result;
    }
    result = ktxTexture_IterateLevels(ktx, optimalTilingCallback, &cbData);
  } else {
    if (ktx->pData) {
      result =
        ktxTexture_IterateLevelFaces(ktx, optimalTilingPadCallback, &cbData);
    } else {
      result = ktxTexture_IterateLoadLevelFaces(ktx, optimalTilingPadCallback,
                                                &cbData);
    }
  }

  if (result == KTX_SUCCESS) {
    rhi::upload(rd, srcStagingBuffer, copyRegions, texture,
                ktx->generateMipmaps);
  }
  return result;
}

[[nodiscard]] std::expected<rhi::Texture, std::string>
loadTexture(rhi::RenderDevice &rd, ktxTexture *ktx) {
  assert(ktx && ktx->baseDepth == 1);

  // https://github.khronos.org/KTX-Software/libktx/annotated.html

  auto numMipLevels = ktx->numLevels;
  auto usageFlags = rhi::ImageUsage::TransferDst | rhi::ImageUsage::Sampled;
  if (ktx->generateMipmaps) {
    numMipLevels = rhi::calcMipLevels(ktx->baseWidth);
    usageFlags |= rhi::ImageUsage::TransferSrc;
  }

  const auto pixelFormat = rhi::PixelFormat(ktxTexture_GetVkFormat(ktx));
  auto texture =
    rhi::Texture::Builder{}
      .setExtent(
        {
          .width = ktx->baseWidth,
          .height = ktx->numDimensions > 1 ? ktx->baseHeight : 0,
        },
        ktx->numDimensions == 3 ? ktx->baseDepth : 0)
      .setPixelFormat(pixelFormat)
      .setNumMipLevels(numMipLevels)
      .setNumLayers(ktx->isArray ? std::optional{ktx->numLayers} : std::nullopt)
      .setCubemap(ktx->isCubemap)
      .setUsageFlags(usageFlags)
      .setupOptimalSampler(true)
      .build(rd);

  if (!texture) {
    return std::unexpected{std::format("Unsupported pixel format: VkFormat({})",
                                       std::to_underlying(pixelFormat))};
  }
  const auto result = upload(rd, texture, ktx);
  if (result != KTX_SUCCESS) {
    return std::unexpected{toString(result)};
  }
  return texture;
}

[[nodiscard]] auto pickTranscodeFormat(const rhi::RenderDevice &rd) {
  if (const auto &df = rd.getDeviceFeatures(); df.textureCompressionETC2) {
    return KTX_TTF_ETC;
  } else if (df.textureCompressionBC) {
    return KTX_TTF_BC3_RGBA;
  }
  // Compressed textures not supported.
  return KTX_TTF_RGBA32;
}

} // namespace

std::expected<rhi::Texture, std::string>
loadTextureKTX(const std::filesystem::path &p, rhi::RenderDevice &rd) {
  auto buffer = os::FileSystem::readBuffer(p);
  if (!buffer) {
    return std::unexpected{buffer.error()};
  }
  struct Deleter {
    void operator()(ktxTexture *ktx) const { ktxTexture_Destroy(ktx); }
  };
  std::unique_ptr<ktxTexture, Deleter> ktx;
  {
    ktxTexture *temp{nullptr};
    const auto result = ktxTexture_CreateFromMemory(
      std::bit_cast<const ktx_uint8_t *>(buffer->data.get()), buffer->size,
      KTX_TEXTURE_CREATE_NO_FLAGS, &temp);
    if (result != KTX_SUCCESS) {
      return std::unexpected{toString(result)};
    }
    assert(temp);
    ktx.reset(temp);
  }

  if (ktxTexture_NeedsTranscoding(ktx.get())) {
    const auto result = ktxTexture2_TranscodeBasis(
      std::bit_cast<ktxTexture2 *>(ktx.get()), pickTranscodeFormat(rd), 0);
    if (result != KTX_SUCCESS) {
      return std::unexpected{toString(result)};
    }
  }
  return loadTexture(rd, ktx.get());
}
