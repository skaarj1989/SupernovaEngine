#pragma once

#if __has_include("nlohmann/json.hpp")

#  include "Material.hpp"
#  include "VisitorHelper.hpp"
#  include "math/json.hpp"

namespace gfx {

#  define MAKE_PAIR(Enum, Value)                                               \
    { Enum::Value, #Value }

NLOHMANN_JSON_SERIALIZE_ENUM(MaterialDomain,
                             {
                               MAKE_PAIR(MaterialDomain, Surface),
                               MAKE_PAIR(MaterialDomain, PostProcess),
                             });
NLOHMANN_JSON_SERIALIZE_ENUM(ShadingModel, {
                                             MAKE_PAIR(ShadingModel, Unlit),
                                             MAKE_PAIR(ShadingModel, Lit),
                                           });
NLOHMANN_JSON_SERIALIZE_ENUM(BlendMode, {
                                          MAKE_PAIR(BlendMode, Opaque),
                                          MAKE_PAIR(BlendMode, Masked),
                                          MAKE_PAIR(BlendMode, Transparent),
                                          MAKE_PAIR(BlendMode, Add),
                                          MAKE_PAIR(BlendMode, Modulate),
                                        });
NLOHMANN_JSON_SERIALIZE_ENUM(BlendOp, {
                                        MAKE_PAIR(BlendOp, Keep),
                                        MAKE_PAIR(BlendOp, Replace),
                                        MAKE_PAIR(BlendOp, Blend),
                                      });
NLOHMANN_JSON_SERIALIZE_ENUM(LightingMode,
                             {
                               MAKE_PAIR(LightingMode, Default),
                               MAKE_PAIR(LightingMode, Transmission),
                             });

#  undef MAKE_PAIR

static void from_json(const nlohmann::json &j, DecalBlendMode &out) {
#  define GET_VALUE(Member) j.at(#Member).get_to(out.Member)

  GET_VALUE(normal);
  GET_VALUE(emissive);
  GET_VALUE(albedo);
  GET_VALUE(metallicRoughnessAO);

#  undef GET_VALUE
}
static void to_json(nlohmann::ordered_json &j, const DecalBlendMode &in) {
#  define STORE_VALUE(Member) {#Member, in.Member}

  j = nlohmann::ordered_json{
    STORE_VALUE(normal),
    STORE_VALUE(emissive),
    STORE_VALUE(albedo),
    STORE_VALUE(metallicRoughnessAO),
  };

#  undef STORE_VALUE
}

} // namespace gfx

// https://www.kdab.com/jsonify-with-nlohmann-json/

template <> struct nlohmann::adl_serializer<gfx::Property::Value> {
  static void from_json(const json &j, gfx::Property::Value &out) {
    const auto type = j.at("type").get<std::string>();
    if (type == "uint")
      out = j.at("value").get<uint32_t>();
    else if (type == "int")
      out = j.at("value").get<int32_t>();
    else if (type == "float")
      out = j.at("value").get<float>();
    else if (type == "vec2")
      out = j.at("value").get<glm::vec2>();
    else if (type == "vec4")
      out = j.at("value").get<glm::vec4>();
    else {
      throw std::runtime_error{
        "non-exhaustive pattern match in: "
        "void from_json(const json &, gfx::Property::Value &)",
      };
    }
  }
  static void to_json(ordered_json &j, const gfx::Property::Value &in) {
#  define STORE(Type, ...)                                                     \
    j["type"] = #Type;                                                         \
    j["value"] = __VA_ARGS__

    std::visit(Overload{
                 [&j](uint32_t v) { STORE(uint, v); },
                 [&j](int32_t v) { STORE(int, v); },
                 [&j](float v) { STORE(float, v); },
                 [&j](glm::vec2 v) {
                   STORE(vec2, {v.x, v.y});
                 },
                 [&j](const glm::vec4 &v) {
                   STORE(vec4, {v.x, v.y, v.z, v.w});
                 },
               },
               in);
#  undef STORE
  }
};

#endif
