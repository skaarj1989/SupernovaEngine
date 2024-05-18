#include "rhi/PixelFormat.hpp"
#include <cassert>

namespace rhi {

uint8_t getBytesPerPixel(const rhi::PixelFormat format) {
  switch (format) {
    using enum rhi::PixelFormat;

    //
    // 8 bits per component:
    //

  case R8_UNorm:
  case R8_SNorm:
  case R8UI:
  case R8I:
    return 1;

  case RG8_UNorm:
  case RG8_SNorm:
  case RG8UI:
  case RG8I:
    return 2;

  case RGBA8_UNorm:
  case RGBA8_sRGB:
  case BGRA8_UNorm:
  case BGRA8_sRGB:
  case RGBA8UI:
  case RGBA8I:
    return 4;

    //
    // 16 bits per component:
    //

  case R16_UNorm:
  case R16_SNorm:
  case R16F:
  case R16UI:
  case R16I:
    return 2;

  case RG16_UNorm:
  case RG16_SNorm:
  case RG16F:
  case RG16UI:
  case RG16I:
    return 4;

  case RGBA16_UNorm:
  case RGBA16_SNorm:
  case RGBA16F:
  case RGBA16UI:
  case RGBA16I:
    return 8;

    //
    // 32 bits per component:
    //

  case R32F:
  case R32UI:
  case R32I:
    return 4;

  case RG32F:
  case RG32UI:
  case RG32I:
    return 8;

  case RGBA32F:
  case RGBA32UI:
  case RGBA32I:
    return 16;

    //
    // Depth/Stencil:
    //

  case Depth16:
    return 2;
  case Depth32F:
    return 4;
  case Stencil8:
    return 1;
  case Depth16_Stencil8:
    return 3;
  case Depth24_Stencil8:
    return 4;
  case Depth32F_Stencil8:
    return 5;

  default:
    return 0;
  }
}

VkImageAspectFlags getAspectMask(const PixelFormat pixelFormat) {
  switch (pixelFormat) {
    using enum PixelFormat;

  case Undefined:
    return VK_IMAGE_ASPECT_NONE;

  case Depth16:
  case Depth32F:
    return VK_IMAGE_ASPECT_DEPTH_BIT;

  case Stencil8:
    return VK_IMAGE_ASPECT_STENCIL_BIT;

  case Depth16_Stencil8:
  case Depth24_Stencil8:
  case Depth32F_Stencil8:
    return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

  default:
    return VK_IMAGE_ASPECT_COLOR_BIT;
  }
}

#define CASE(Value)                                                            \
  case Value:                                                                  \
    return #Value

const char *toString(const PixelFormat pixelFormat) {
  switch (pixelFormat) {
    using enum PixelFormat;

    // -- Normalized float:

    CASE(R8_UNorm);
    CASE(RG8_UNorm);
    CASE(RGBA8_UNorm);
    CASE(RGBA8_sRGB);

    CASE(R8_SNorm);
    CASE(RG8_SNorm);
    CASE(RGBA8_SNorm);

    CASE(BGRA8_UNorm);
    CASE(BGRA8_sRGB);

    // -- Integer:

    CASE(R8UI);
    CASE(RG8UI);
    CASE(RGBA8UI);

    CASE(R8I);
    CASE(RG8I);
    CASE(RGBA8I);

    //
    // 16 bits per component:
    //

    // -- Normalized float:

    CASE(R16_UNorm);
    CASE(RG16_UNorm);
    CASE(RGBA16_UNorm);

    CASE(R16_SNorm);
    CASE(RG16_SNorm);
    CASE(RGBA16_SNorm);

    // -- Float:

    CASE(R16F);
    CASE(RG16F);
    CASE(RGBA16F);

    // -- Integer:

    CASE(R16UI);
    CASE(RG16UI);
    CASE(RGBA16UI);

    CASE(R16I);
    CASE(RG16I);
    CASE(RGBA16I);

    //
    // 32 bits per component:
    //

    // -- Float:

    CASE(R32F);
    CASE(RG32F);
    CASE(RGBA32F);

    // -- Integer:

    CASE(R32UI);
    CASE(RG32UI);
    CASE(RGBA32UI);

    CASE(R32I);
    CASE(RG32I);
    CASE(RGBA32I);

    //
    // ETC:
    //

    CASE(ETC2_RGB8_UNorm);
    CASE(ETC2_RGBA8_UNorm);
    CASE(ETC2_RGB8A1_UNorm);

    CASE(ETC2_RGB8_sRGB);
    CASE(ETC2_RGBA8_sRGB);
    CASE(ETC2_RGB8A1_sRGB);

    CASE(BC2_UNorm);
    CASE(BC2_sRGB);

    //
    // BC3:
    //

    CASE(BC3_UNorm);
    CASE(BC3_sRGB);

    //
    // Depth/Stencil:
    //

    CASE(Depth16);
    CASE(Depth32F);

    CASE(Stencil8);

    CASE(Depth16_Stencil8);
    CASE(Depth24_Stencil8);
    CASE(Depth32F_Stencil8);

  default:
    assert(false);
    return "Undefined";
  }
}

} // namespace rhi
