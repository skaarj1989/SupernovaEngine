#include "LuaVulkanRHI.hpp"
#include "sol/state.hpp"

#include "rhi/RenderDevice.hpp"
#include "Sol2HelperMacros.hpp"

namespace {

void registerExtent2D(sol::state &lua) {
  using rhi::Extent2D;

#define BIND(Member) _BIND(Extent2D, Member)
  // clang-format off
  DEFINE_USERTYPE(Extent2D,
    sol::call_constructor,
    sol::constructors<
      Extent2D(),
      Extent2D(uint32_t, uint32_t)
    >(),

    BIND(width),
    BIND(height),

    sol::meta_function::to_string,
      [](const Extent2D &self) {
        return std::format("Extent2D({}, {})", self.width, self.height);
      }
  );
  // clang-format on
#undef BIND
}

void registerTextureType(sol::state &lua) {
  using rhi::TextureType;

#define MAKE_PAIR(Value) _MAKE_PAIR(TextureType, Value)
  DEFINE_ENUM(TextureType, {
                             MAKE_PAIR(Undefined),
                             MAKE_PAIR(Texture1D),
                             MAKE_PAIR(Texture1DArray),
                             MAKE_PAIR(Texture2D),
                             MAKE_PAIR(Texture2DArray),
                             MAKE_PAIR(Texture3D),
                             MAKE_PAIR(TextureCube),
                             MAKE_PAIR(TextureCubeArray),
                           });
#undef MAKE_PAIR
}
void registerPixelFormat(sol::state &lua) {
  using rhi::PixelFormat;

#define MAKE_PAIR(Value) _MAKE_PAIR(PixelFormat, Value)
  DEFINE_ENUM(PixelFormat, {
                             MAKE_PAIR(Undefined),

                             MAKE_PAIR(R8_UNorm),
                             MAKE_PAIR(RG8_UNorm),
                             MAKE_PAIR(RGBA8_UNorm),
                             MAKE_PAIR(RGBA8_sRGB),

                             MAKE_PAIR(R8_SNorm),
                             MAKE_PAIR(RG8_SNorm),
                             MAKE_PAIR(RGBA8_SNorm),

                             MAKE_PAIR(BGRA8_UNorm),
                             MAKE_PAIR(BGRA8_sRGB),

                             MAKE_PAIR(R8UI),
                             MAKE_PAIR(RG8UI),
                             MAKE_PAIR(RGBA8UI),

                             MAKE_PAIR(R8I),
                             MAKE_PAIR(RG8I),
                             MAKE_PAIR(RGBA8I),

                             MAKE_PAIR(R16_UNorm),
                             MAKE_PAIR(RG16_UNorm),
                             MAKE_PAIR(RGBA16_UNorm),

                             MAKE_PAIR(R16_SNorm),
                             MAKE_PAIR(RG16_SNorm),
                             MAKE_PAIR(RGBA16_SNorm),

                             MAKE_PAIR(R16F),
                             MAKE_PAIR(RG16F),
                             MAKE_PAIR(RGBA16F),

                             MAKE_PAIR(R16UI),
                             MAKE_PAIR(RG16UI),
                             MAKE_PAIR(RGBA16UI),

                             MAKE_PAIR(R16I),
                             MAKE_PAIR(RG16I),
                             MAKE_PAIR(RGBA16I),

                             MAKE_PAIR(R32F),
                             MAKE_PAIR(RG32F),
                             MAKE_PAIR(RGBA32F),

                             MAKE_PAIR(R32UI),
                             MAKE_PAIR(RG32UI),
                             MAKE_PAIR(RGBA32UI),

                             MAKE_PAIR(R32I),
                             MAKE_PAIR(RG32I),
                             MAKE_PAIR(RGBA32I),

                             MAKE_PAIR(ETC2_RGB8_UNorm),
                             MAKE_PAIR(ETC2_RGBA8_UNorm),
                             MAKE_PAIR(ETC2_RGB8A1_UNorm),

                             MAKE_PAIR(ETC2_RGB8_sRGB),
                             MAKE_PAIR(ETC2_RGBA8_sRGB),
                             MAKE_PAIR(ETC2_RGB8A1_sRGB),

                             MAKE_PAIR(BC2_UNorm),
                             MAKE_PAIR(BC2_sRGB),

                             MAKE_PAIR(BC3_UNorm),
                             MAKE_PAIR(BC3_sRGB),

                             MAKE_PAIR(Depth16),
                             MAKE_PAIR(Depth32F),

                             MAKE_PAIR(Stencil8),

                             MAKE_PAIR(Depth16_Stencil8),
                             MAKE_PAIR(Depth24_Stencil8),
                             MAKE_PAIR(Depth32F_Stencil8),
                           });
#undef MAKE_PAIR
}

void registerTexture(sol::state &lua) {
  registerTextureType(lua);
  registerPixelFormat(lua);

  using rhi::Texture;

#define BIND(Member) _BIND(Texture, Member)
  // clang-format off
  DEFINE_USERTYPE(Texture,
    sol::no_constructor,

    BIND(getType),
    BIND(getExtent),
    BIND(getDepth),
    BIND(getNumMipLevels),
    BIND(getNumLayers),
    BIND(getPixelFormat),

    BIND_TOSTRING(Texture)
  );
  // clang-format on
#undef BIND
}

} // namespace

void registerVulkanRHI(sol::state &lua) {
  registerExtent2D(lua);
  registerTexture(lua);
}
