#pragma once

#if __has_include("nlohmann/json.hpp")

#  include "VertexFormat.hpp"
#  include "nlohmann/json.hpp"

#  if defined(_MSC_VER)
#    pragma warning(push)
#    pragma warning(disable : 4505) // unreferenced function with internal
                                    // linkage has been removed
#  elif defined(__GNUC__)
#    pragma GCC diagnostic push
#    pragma GCC diagnostic ignored "-Wunused-function"
#  endif

#  define MAKE_PAIR(Value)                                                     \
    { AttributeLocation::Value, #Value }

namespace gfx {

// clang-format off
NLOHMANN_JSON_SERIALIZE_ENUM(AttributeLocation, {
  MAKE_PAIR(Position),
  MAKE_PAIR(Color_0),
  MAKE_PAIR(Normal),
  MAKE_PAIR(TexCoord_0),
  MAKE_PAIR(TexCoord_1),
  MAKE_PAIR(Tangent),
  MAKE_PAIR(Bitangent),
  MAKE_PAIR(Joints),
  MAKE_PAIR(Weights),
});
// clang-format on

} // namespace gfx

#  undef MAKE_PAIR

#  if defined(_MSC_VER)
#    pragma warning(pop)
#  elif defined(__GNUC__)
#    pragma GCC diagnostic pop
#  endif

#endif
