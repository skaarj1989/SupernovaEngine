#pragma once

#if __has_include("nlohmann/json.hpp")

#  include "VertexFormat.hpp"
#  include "nlohmann/json.hpp"

#  define MAKE_PAIR(Value)                                                     \
    { AttributeLocation::Value, #Value }

namespace gfx {

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

} // namespace gfx

#  undef MAKE_PAIR

#endif
