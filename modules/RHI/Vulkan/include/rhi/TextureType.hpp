#pragma once

namespace rhi {

enum class TextureType {
  Undefined = 0,

  Texture1D,
  Texture1DArray,
  Texture2D,
  Texture2DArray,
  Texture3D,
  TextureCube,
  TextureCubeArray,
};

[[nodiscard]] const char *toString(const TextureType);

} // namespace rhi
