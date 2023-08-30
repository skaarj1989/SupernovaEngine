#pragma once

#if __has_include("nlohmann/json.hpp")

#  include "math/AABB.hpp"
#  include "glm/vec2.hpp"
#  include "glm/vec3.hpp"
#  include "glm/vec4.hpp"
#  include "glm/gtc/quaternion.hpp"
#  include "nlohmann/json.hpp"

namespace glm {

template <length_t L, typename T, qualifier Q = defaultp>
static void from_json(const nlohmann::json &j, vec<L, T, Q> &out) {
  if (j.is_array()) {
    // "key": [1, 0, ...]
    for (auto i = 0; i < L; ++i) {
      out[i] = j[i].get<T>();
    }
  } else if (j.is_object()) {
    /*
      "key": {
        "x": 0.0,
        "y": 0.0,
        ...
      }
    */
    constexpr auto kComponents = std::array{"x", "y", "z", "w"};
    for (auto i = 0; i < L; ++i) {
      out[i] = j.at(kComponents[i]).get<T>();
    }
  }
}

static void to_json(nlohmann::ordered_json &j, const vec2 &in) {
  j = nlohmann::ordered_json{in.x, in.y};
}
static void to_json(nlohmann::ordered_json &j, const vec3 &in) {
  j = nlohmann::ordered_json{in.x, in.y, in.z};
}
static void to_json(nlohmann::ordered_json &j, const vec4 &in) {
  j = nlohmann::ordered_json{in.x, in.y, in.z, in.w};
}

static void from_json(const nlohmann::json &j, quat &out) {
  using T = std::decay_t<decltype(out)>;
  if (j.is_array()) {
    for (auto i = 0; i < T::length(); ++i) {
      out[i] = j[i].get<float>();
    }
  } else if (j.is_object()) {
    constexpr auto kComponents = std::array{
#  ifdef GLM_FORCE_QUAT_DATA_XYZW
      "x", "y", "z", "w"
#  else
      "w", "x", "y", "z"
#  endif
    };
    for (auto i = 0; i < T::length(); ++i) {
      out[i] = j.at(kComponents[i]).get<float>();
    }
  }
}
static void to_json(nlohmann::ordered_json &j, const quat &q) {
  j = nlohmann::ordered_json{q.x, q.y, q.z, q.w};
}

} // namespace glm

static void from_json(const nlohmann::json &j, AABB &out) {
  j.at("min").get_to(out.min);
  j.at("max").get_to(out.max);
}
static void to_json(nlohmann::ordered_json &j, const AABB &in) {
  j = nlohmann::ordered_json{
    {"min", in.min},
    {"max", in.max},
  };
}

#endif
