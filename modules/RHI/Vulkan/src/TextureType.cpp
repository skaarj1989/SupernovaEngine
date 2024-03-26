#include "rhi/TextureType.hpp"
#include <cassert>

namespace rhi {

const char *toString(const TextureType textureType) {
#define CASE(Value)                                                            \
  case Value:                                                                  \
    return #Value

  switch (textureType) {
    using enum TextureType;

    CASE(Texture1D);
    CASE(Texture1DArray);
    CASE(Texture2D);
    CASE(Texture2DArray);
    CASE(Texture3D);
    CASE(TextureCube);
    CASE(TextureCubeArray);
  }
  return "Undefined";
}

} // namespace rhi
