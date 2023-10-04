---@meta

---@enum ShaderType
ShaderType = {
  Vertex = 0, Geometry = 1, Fragment = 2, Compute = 3
}

---@enum CullMode
CullMode = {
  None = 0, Front = 1, Back = 2
}

---@class Extent2D
---@field width integer
---@field height integer
---@overload fun(): Extent2D
---@overload fun(width: integer, height: integer): Extent2D
Extent2D = {}

---@class Texture
Texture = {}

---@return TextureType
function Texture:getType() end

---@return Extent2D
function Texture:getExtent() end

---@return integer
function Texture:getDepth() end

---@return integer
function Texture:getNumMipLevels() end

---@return integer
function Texture:getNumLayers() end

---@return PixelFormat
function Texture:getPixelFormat() end

---@enum TextureType
TextureType = {
  Undefined = 0,
  Texture1D = 1,
  Texture1DArray = 2,
  Texture2D = 3,
  Texture2DArray = 4,
  Texture3D = 5,
  TextureCube = 6,
  TextureCubeArray = 7,
}

---@enum PixelFormat
PixelFormat = {
  Undefined = 0,

  R8_UNorm = 9,
  RG8_UNorm = 16,
  RGBA8_UNorm = 37,
  RGBA8_sRGB = 43,

  R8_SNorm = 10,
  RG8_SNorm = 17,
  RGBA8_SNorm = 38,

  BGRA8_UNorm = 44,
  BGRA8_sRGB = 50,

  R8UI = 13,
  RG8UI = 20,
  RGBA8UI = 41,

  R8I = 14,
  RG8I = 21,
  RGBA8I = 42,

  R16_UNorm = 70,
  RG16_UNorm = 77,
  RGBA16_UNorm = 91,

  R16_SNorm = 71,
  RG16_SNorm = 78,
  RGBA16_SNorm = 92,

  R16F = 76,
  RG16F = 83,
  RGBA16F = 97,

  R16UI = 74,
  RG16UI = 81,
  RGBA16UI = 95,

  R16I = 75,
  RG16I = 82,
  RGBA16I = 96,

  R32F = 100,
  RG32F = 103,
  RGBA32F = 109,

  R32UI = 98,
  RG32UI = 101,
  RGBA32UI = 107,

  R32I = 99,
  RG32I = 102,
  RGBA32I = 108,

  ETC2_RGB8_UNorm = 147,
  ETC2_RGBA8_UNorm = 151,
  ETC2_RGB8A1_UNorm = 149,

  ETC2_RGB8_sRGB = 148,
  ETC2_RGBA8_sRGB = 152,
  ETC2_RGB8A1_sRGB = 150,

  BC2_UNorm = 135,
  BC2_sRGB = 136,

  BC3_UNorm = 137,
  BC3_sRGB = 138,

  Depth16 = 124,
  Depth32F = 126,

  Stencil8 = 127,

  Depth16_Stencil8 = 128,
  Depth24_Stencil8 = 129,
  Depth32F_Stencil8 = 130,
}
