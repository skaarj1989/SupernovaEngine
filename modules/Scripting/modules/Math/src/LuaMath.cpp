#include "LuaMath.hpp"
#include "sol/state.hpp"

#include "math/Frustum.hpp"
#include "math/AABB.hpp"
#include "math/Sphere.hpp"
#include "math/Cone.hpp"

#include "glm/gtc/constants.hpp"  // pi, half_pi, epsilon
#include "glm/gtc/quaternion.hpp" // quat
#include "glm/gtc/noise.hpp"      // perlin

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/norm.hpp"          // length2
#include "glm/gtx/perpendicular.hpp" // perp

#include "Sol2HelperMacros.hpp"

#include <format>

#define GEN_TYPE_X(fn)                                                         \
  sol::overload(sol::resolve<float(float)>(fn),                                \
                sol::resolve<glm::vec2(const glm::vec2 &)>(fn),                \
                sol::resolve<glm::vec3(const glm::vec3 &)>(fn),                \
                sol::resolve<glm::vec4(const glm::vec4 &)>(fn))
#define GEN_TYPE_X2(fn)                                                        \
  sol::overload(                                                               \
    sol::resolve<float(float, float)>(fn),                                     \
    sol::resolve<glm::vec2(const glm::vec2 &, const glm::vec2 &)>(fn),         \
    sol::resolve<glm::vec3(const glm::vec3 &, const glm::vec3 &)>(fn))
#define GEN_TYPE_X3(fn)                                                        \
  sol::overload(sol::resolve<float(float, float, float)>(fn),                  \
                sol::resolve<glm::vec2(const glm::vec2 &, const glm::vec2 &,   \
                                       const glm::vec2 &)>(fn),                \
                sol::resolve<glm::vec3(const glm::vec3 &, const glm::vec3 &,   \
                                       const glm::vec3 &)>(fn))
#define GEN_TYPE_OP(T, op)                                                     \
  sol::overload(sol::resolve<T(const T &, const T &)>(op),                     \
                sol::resolve<T(const T &, typename T::value_type)>(op),        \
                sol::resolve<T(typename T::value_type, const T &)>(op))

namespace {

// https://glm.g-truc.net/0.9.9/api/a00280.html

void registerConstants(sol::table &lua) {
  // Accessed like a function call.
  lua["PI"] = sol::readonly_property(&glm::pi<float>);
  lua["HALF_PI"] = sol::readonly_property(&glm::half_pi<float>);
  lua["EPSILON"] = sol::readonly_property(&glm::epsilon<float>);
}
void registerFreeFunctions(sol::table &lua) {
#pragma region common
  lua.set_function("abs", GEN_TYPE_X(glm::abs));
  lua.set_function("floor", GEN_TYPE_X(glm::floor));
  lua.set_function("ceil", GEN_TYPE_X(glm::ceil));

  lua.set_function("min", GEN_TYPE_X2(glm::min));
  lua.set_function("max", GEN_TYPE_X2(glm::max));
  lua.set_function("clamp", GEN_TYPE_X3(glm::clamp));

  lua.set_function("mix", GEN_TYPE_X3(glm::mix));
#pragma endregion
#pragma region trigonometric
  lua.set_function("degrees", GEN_TYPE_X(glm::degrees));
  lua.set_function("radians", GEN_TYPE_X(glm::radians));

  lua.set_function("sin", sol::resolve<float(float)>(glm::sin));
  lua.set_function("cos", sol::resolve<float(float)>(glm::cos));
  lua.set_function("tan", sol::resolve<float(float)>(glm::tan));
#pragma endregion
#pragma region exponential
  lua.set_function("exp", GEN_TYPE_X(glm::exp));
  lua.set_function("exp2", GEN_TYPE_X(glm::exp2));
  lua.set_function("inversesqrt", GEN_TYPE_X(glm::inversesqrt));
  lua.set_function("log", GEN_TYPE_X(glm::log));
  lua.set_function("log2", GEN_TYPE_X(glm::log2));
  lua.set_function("pow", GEN_TYPE_X2(glm::pow));
  lua.set_function("sqrt", GEN_TYPE_X(glm::sqrt));
#pragma endregion
#pragma region geometric
  lua.set_function(
    "dot",
    sol::overload(
      sol::resolve<float(const glm::vec2 &, const glm::vec2 &)>(glm::dot),
      sol::resolve<float(const glm::vec3 &, const glm::vec3 &)>(glm::dot),
      sol::resolve<float(const glm::vec4 &, const glm::vec4 &)>(glm::dot),
      sol::resolve<float(const glm::quat &, const glm::quat &)>(glm::dot)));
  lua.set_function(
    "cross",
    sol::overload(
      sol::resolve<glm::vec3(const glm::vec3 &, const glm::vec3 &)>(glm::cross),
      sol::resolve<glm::quat(const glm::quat &, const glm::quat &)>(
        glm::cross)));

  lua.set_function(
    "distance",
    sol::overload(
      sol::resolve<float(const glm::vec2 &, const glm::vec2 &)>(glm::distance),
      sol::resolve<float(const glm::vec3 &, const glm::vec3 &)>(
        glm::distance)));
  lua.set_function(
    "length",
    sol::overload(sol::resolve<float(const glm::vec2 &)>(glm::length),
                  sol::resolve<float(const glm::vec3 &)>(glm::length)));

  // clang-format off
  lua.set_function(
    "reflect", 
    sol::overload(
      sol::resolve<glm::vec2(const glm::vec2 &, const glm::vec2 &)>(glm::reflect),
      sol::resolve<glm::vec3(const glm::vec3 &, const glm::vec3 &)>(glm::reflect)
    )
  );
  lua.set_function(
    "refract", 
    sol::overload(
      sol::resolve<glm::vec2(const glm::vec2 &, const glm::vec2 &, float)>(glm::refract),
      sol::resolve<glm::vec3(const glm::vec3 &, const glm::vec3 &, float)>(glm::refract)
    )
  );
  // clang-format on

  lua.set_function(
    "normalize",
    sol::overload(sol::resolve<glm::vec2(const glm::vec2 &)>(glm::normalize),
                  sol::resolve<glm::vec3(const glm::vec3 &)>(glm::normalize),
                  sol::resolve<glm::vec4(const glm::vec4 &)>(glm::normalize),
                  sol::resolve<glm::quat(const glm::quat &)>(glm::normalize)));
#pragma endregion
#pragma region matrix
  lua.set_function("inverse",
                   sol::resolve<glm::mat4(const glm::mat4 &)>(glm::inverse));
  lua.set_function("transpose",
                   sol::resolve<glm::mat4(const glm::mat4 &)>(glm::transpose));
#pragma endregion
#pragma region noise
  lua.set_function(
    "noise", sol::overload(sol::resolve<float(const glm::vec2 &)>(glm::perlin),
                           [](float x, float y) {
                             return glm::perlin(glm::vec2{x, y});
                           }));
#pragma endregion
}

template <typename T> void addArithmeticOperators(sol::usertype<T> &type) {
  type.set_function(sol::meta_function::addition,
                    GEN_TYPE_OP(T, glm::operator+));
  type.set_function(sol::meta_function::subtraction,
                    GEN_TYPE_OP(T, glm::operator-));
  type.set_function(sol::meta_function::multiplication,
                    GEN_TYPE_OP(T, glm::operator*));
  type.set_function(sol::meta_function::division,
                    GEN_TYPE_OP(T, glm::operator/));
}

template <typename T, bool arithmeticOperators = true>
void registerVec2(const char *name, sol::table &lua) {
  static_assert(T::length() == 2);

  using value_type = T::value_type;

  // clang-format off
  auto type = lua.new_usertype<T>(
    name, sol::call_constructor,
    sol::constructors<
      T(),
      T(value_type),
      T(value_type, value_type),
      T(const glm::vec2 &),
      T(const glm::vec3 &),
      T(const glm::vec4 &),
      T(const glm::ivec2 &),
      T(const glm::ivec3 &),
      T(const glm::ivec4 &),
      T(const glm::uvec2 &),
      T(const glm::uvec3 &),
      T(const glm::uvec4 &)
    >(),

    "x", &T::x, "y", &T::y,
    
    "makeArray", [] { return std::vector<T>{}; },
    
    sol::meta_function::to_string,
      [name](const T &v) { return std::format("{}({}, {})", name, v.x, v.y); }
  );
  // clang-format on

  if constexpr (arithmeticOperators) {
    addArithmeticOperators(type);
  }
}

template <typename T, bool arithmeticOperators = true>
void registerVec3(const char *name, sol::table &lua) {
  static_assert(T::length() == 3);

  using value_type = T::value_type;

  // clang-format off
  auto type = lua.new_usertype<T>(
    name, sol::call_constructor,
    sol::constructors<
      T(),
      T(value_type),
      T(value_type, value_type, value_type),
      T(const glm::vec3 &),
      T(const glm::vec4 &),
      T(const glm::ivec3 &),
      T(const glm::ivec4 &),
      T(const glm::uvec3 &),
      T(const glm::uvec4 &)
    >(),

    "x", &T::x, "y", &T::y, "z", &T::z,
    "r", &T::r, "g", &T::g, "b", &T::b,
    
    "makeArray", [] { return std::vector<T>{}; },
    
    sol::meta_function::to_string,
      [name](const T &v) { return std::format("{}({}, {}, {})", name, v.x, v.y, v.z); }
  );
  // clang-format on

  if constexpr (arithmeticOperators) {
    addArithmeticOperators(type);
  }
}

void registerVec4(sol::table &lua) {
  // clang-format off
  lua.new_usertype<glm::vec4>("vec4",
    // vec4(...)
    sol::call_constructor,
    // vec4.new(...)
    sol::constructors<
      glm::vec4(),
      glm::vec4(const glm::vec4 &),
      glm::vec4(float),
      glm::vec4(float, float, float, float),
      glm::vec4(const glm::vec2 &, const glm::vec2 &),
      glm::vec4(const glm::vec2 &, float, float),
      glm::vec4(const glm::vec3 &, float)
    >(),
    
    "x", &glm::vec4::x, "r", &glm::vec4::r,
    "y", &glm::vec4::y, "g", &glm::vec4::g,
    "z", &glm::vec4::z, "b", &glm::vec4::b,
    "w", &glm::vec4::w, "a", &glm::vec4::a,

    "makeArray", [] { return std::vector<glm::vec4>{}; },

    // glm/detail/type_vec4.inl

    sol::meta_function::addition,
      sol::overload(
        sol::resolve<glm::vec4(const glm::vec4 &, const float)>(glm::operator+),
        sol::resolve<glm::vec4(float, const glm::vec4 &)>(glm::operator+),
        sol::resolve<glm::vec4(const glm::vec4 &, const glm::vec4 &)>(glm::operator+)
      ),
    sol::meta_function::subtraction,
      sol::overload(
        sol::resolve<glm::vec4(const glm::vec4 &, const float)>(glm::operator-),
        sol::resolve<glm::vec4(float, const glm::vec4 &)>(glm::operator-),
        sol::resolve<glm::vec4(const glm::vec4 &, const glm::vec4 &)>(glm::operator-)
      ),
    sol::meta_function::multiplication,
      sol::overload(
        sol::resolve<glm::vec4(const glm::vec4 &, const float)>(glm::operator*),
        sol::resolve<glm::vec4(float, const glm::vec4 &)>(glm::operator*),
        sol::resolve<glm::vec4(const glm::vec4 &, const glm::vec4 &)>(glm::operator*)
      ),
    sol::meta_function::division,
      sol::overload(
        sol::resolve<glm::vec4(const glm::vec4 &, const float)>(glm::operator/),
        sol::resolve<glm::vec4(float, const glm::vec4 &)>(glm::operator/),
        sol::resolve<glm::vec4(const glm::vec4 &, const glm::vec4 &)>(glm::operator/)
      ),

    sol::meta_function::to_string, [](const glm::vec4 &v) {
      return std::format("vec4({}, {}, {}, {})", v.x, v.y, v.z, v.w);
    }
  );
  // clang-format on
}

void registerMat4(sol::table &lua) {
  // clang-format off
  lua.new_usertype<glm::mat4>("mat4",
    // mat4(...)
    sol::call_constructor,
    // mat4.new(...)
    sol::constructors<
      glm::mat4(),
      glm::mat4(float),
      glm::mat4(
        const glm::vec4 &,
        const glm::vec4 &,
        const glm::vec4 &,
        const glm::vec4 &
      ),
      glm::mat4(
        float, float, float, float,
        float, float, float, float,
        float, float, float, float,
        float, float, float, float
      ),
      glm::mat4(const glm::mat4 &)
    >(),

    "identity", glm::identity<glm::mat4>,

    // glm/detail/type_mat4x4.inl

    sol::meta_function::addition,
      sol::overload(
        sol::resolve<glm::mat4(const glm::mat4 &, const float)>(glm::operator+),                    
        sol::resolve<glm::mat4(const float, const glm::mat4 &)>(glm::operator+),
        sol::resolve<glm::mat4(const glm::mat4 &, const glm::mat4 &)>(glm::operator+)                     
      ),
    sol::meta_function::subtraction,
      sol::overload(
        sol::resolve<glm::mat4(const glm::mat4 &, const float)>(glm::operator-),                    
        sol::resolve<glm::mat4(const float, const glm::mat4 &)>(glm::operator-),
        sol::resolve<glm::mat4(const glm::mat4 &, const glm::mat4 &)>(glm::operator-)                     
      ),
    sol::meta_function::multiplication,
      sol::overload(
        sol::resolve<glm::mat4(const glm::mat4 &, const float)>(glm::operator*),                    
        sol::resolve<glm::mat4(const float, const glm::mat4 &)>(glm::operator*),
        sol::resolve<glm::vec4(const glm::mat4 &, const glm::vec4 &)>(glm::operator*),                    
        sol::resolve<glm::vec4(const glm::vec4 &, const glm::mat4 &)>(glm::operator*),
        sol::resolve<glm::mat4(const glm::mat4 &, const glm::mat4 &)>(glm::operator*)                     
      ),
    sol::meta_function::division,
      sol::overload(
        sol::resolve<glm::mat4(const glm::mat4 &, const float)>(glm::operator/),                    
        sol::resolve<glm::mat4(const float, const glm::mat4 &)>(glm::operator/),
        sol::resolve<glm::vec4(const glm::mat4 &, const glm::vec4 &)>(glm::operator/),                    
        sol::resolve<glm::vec4(const glm::vec4 &, const glm::mat4 &)>(glm::operator/),
        sol::resolve<glm::mat4(const glm::mat4 &, const glm::mat4 &)>(glm::operator/)  
      ),

    sol::meta_function::to_string, [](const glm::mat4 &) {
      return "mat4()";
    }
  );
  // clang-format on  
}

void registerQuat(sol::table &lua) {
  // clang-format off
  lua.new_usertype<glm::quat>("quat",
    // quat(...)
    sol::call_constructor,
    // qut.new(...)
    sol::constructors<
      glm::quat(),
      glm::quat(const glm::quat &),
      glm::quat(float /* w */, float, float, float),
      glm::quat(const glm::vec3 &/* Euler angles. */),
      glm::quat(float, const glm::vec3 &)
    >(),
    
    "x", &glm::quat::x,
    "y", &glm::quat::y,
    "z", &glm::quat::z,
    "w", &glm::quat::w,

    "identity", glm::identity<glm::quat>,

    // glm/detail/type_quat.inl

    sol::meta_function::multiplication,
      sol::overload(
        sol::resolve<glm::quat(const glm::quat &, const glm::quat &)>(glm::operator*),
        sol::resolve<glm::vec3(const glm::quat &, const glm::vec3 &)>(glm::operator*),
        sol::resolve<glm::vec3(const glm::vec3 &, const glm::quat &)>(glm::operator*),
        sol::resolve<glm::vec4(const glm::quat &, const glm::vec4 &)>(glm::operator*),
        sol::resolve<glm::vec4(const glm::vec4 &, const glm::quat &)>(glm::operator*),
        sol::resolve<glm::quat(const glm::quat &, const float &)>(glm::operator*),
        sol::resolve<glm::quat(const float &, const glm::quat &)>(glm::operator*)
      ),
    sol::meta_function::addition,
      sol::overload(
        sol::resolve<glm::quat(const glm::quat &, const glm::quat &)>(glm::operator+)
      ),
    sol::meta_function::subtraction,
      sol::overload(
        sol::resolve<glm::quat(const glm::quat &, const glm::quat &)>(glm::operator-)
      ),
    sol::meta_function::division,
      sol::overload(
        sol::resolve<glm::quat(const glm::quat &, const float &)>(glm::operator/)
      ),

    sol::meta_function::to_string, [](const glm::quat &v) {
      return std::format("quat(w={}, {}, {}, {})", v.w, v.x, v.y, v.z);
    }
  );
  // clang-format on

  lua["eulerAngles"] =
    sol::resolve<glm::vec3(const glm::quat &)>(glm::eulerAngles);
  lua["angleAxis"] =
    sol::resolve<glm::quat(const float &, const glm::vec3 &)>(glm::angleAxis);
  lua["lookAt"] = sol::resolve<glm::quat(const glm::vec3 &, const glm::vec3 &)>(
    glm::quatLookAt);
}

void registerShapes(sol::table &lua) {
#define BIND(Member) _BIND(AABB, Member)
  // clang-format off
  lua.DEFINE_USERTYPE(AABB, 
    sol::call_constructor,
    sol::factories(
      [](const glm::vec3 &min, const glm::vec3 &max) { return AABB{min, max}; }
    ),

    BIND(min),
    BIND(max),

    BIND(getCenter),
    BIND(getExtent),
    BIND(getRadius),

    BIND(transform),

    BIND_TOSTRING(AABB)
  );
#undef BIND

#define BIND(Member) _BIND(Sphere, Member)
  lua.DEFINE_USERTYPE(Sphere, 
    sol::call_constructor,
    sol::factories([](const glm::vec3 &c, float r) { return Sphere{c, r}; }),

    BIND(c),
    BIND(r),

    BIND_TOSTRING(Sphere)
  );
#undef BIND

#define BIND(Member) _BIND(Cone, Member)
  lua.DEFINE_USERTYPE(Cone,
    sol::call_constructor,
    sol::factories(
      [](const glm::vec3 &T, float h, const glm::vec3 &d, float r) {
        return Cone{T, h, d, r};
      }
    ),
    
    BIND(T),
    BIND(h),
    BIND(d),
    BIND(r),

    BIND_TOSTRING(Cone)
  );
  // clang-format on
#undef BIND
}
void registerFrustum(sol::table &lua) {
#define BIND(Member) _BIND(Frustum, Member)
  // clang-format off
  lua.DEFINE_USERTYPE(Frustum,
    sol::no_constructor,

    BIND(testPoint),
    BIND(testAABB),
    BIND(testSphere),
    BIND(testCone),

    BIND_TOSTRING(Frustum)
  );
  // clang-format on
#undef BIND
}

} // namespace

void registerMath(sol::state &lua) {
  auto m = lua["math"].get_or_create<sol::table>();

  registerConstants(m);

  registerVec2<glm::bvec2, false>("bvec2", m);
  registerVec2<glm::vec2>("vec2", m);
  registerVec2<glm::ivec2>("ivec2", m);
  registerVec2<glm::uvec2>("uvec2", m);

  registerVec3<glm::bvec3, false>("bvec3", m);
  registerVec3<glm::vec3>("vec3", m);
  registerVec3<glm::ivec3>("ivec3", m);
  registerVec3<glm::uvec3>("uvec3", m);

  registerVec4(m);

  registerMat4(m);
  registerQuat(m);

  registerFreeFunctions(m);

  registerShapes(m);
  registerFrustum(m);
}
