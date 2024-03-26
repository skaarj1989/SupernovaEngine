#pragma once

#if __has_include("nlohmann/json.hpp")

#  include "CullMode.hpp"
#  include "TextureType.hpp"
#  include "VertexAttributes.hpp"
#  include "nlohmann/json.hpp"

namespace rhi {

#  define _MAKE_PAIR(Enum, Value)                                              \
    { Enum::Value, #Value }

// clang-format off
#  define MAKE_PAIR(Value) _MAKE_PAIR(CullMode, Value)
NLOHMANN_JSON_SERIALIZE_ENUM(CullMode, {
  MAKE_PAIR(None),
  MAKE_PAIR(Front),
  MAKE_PAIR(Back),
});
#  undef MAKE_PAIR

NLOHMANN_JSON_SERIALIZE_ENUM(TextureType, {
  {TextureType::Texture1D, "sampler1D"},
  {TextureType::Texture1DArray, "sampler1DArray"},
  {TextureType::Texture2D, "sampler2D"},
  {TextureType::Texture2DArray, "sampler2DArray"},
  {TextureType::Texture3D, "sampler3D"},
  {TextureType::TextureCube, "samplerCube"},
  {TextureType::TextureCubeArray,
  "samplerCubeArray"},
});

#  define MAKE_PAIR(Value) _MAKE_PAIR(VertexAttribute::Type, Value)
NLOHMANN_JSON_SERIALIZE_ENUM(VertexAttribute::Type, {
  MAKE_PAIR(Float),
  MAKE_PAIR(Float2),
  MAKE_PAIR(Float3),
  MAKE_PAIR(Float4),
  MAKE_PAIR(Int4),
  MAKE_PAIR(UByte4_Norm),
});
#  undef MAKE_PAIR
// clang-format on

static void from_json(const nlohmann::json &j, VertexAttribute &out) {
  j.at("type").get_to(out.type);
  j.at("offset").get_to(out.offset);
}
static void to_json(nlohmann::ordered_json &j, const VertexAttribute &in) {
  j = nlohmann::ordered_json{
    {"type", in.type},
    {"offset", in.offset},
  };
}

} // namespace rhi

#endif
