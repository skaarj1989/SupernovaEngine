#include "DataModel.hpp"
#include "RmlUi/Core/DataModelHandle.h"
#include "RmlUi/Core/Event.h"
#include "RmlUi/Core/Context.h"

#include "entt/core/type_traits.hpp"
#include "Rml2glm.hpp"
#include "glm2Rml.hpp"

#include "Sol2HelperMacros.hpp"

#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wswitch"
#endif

using namespace Rml;

namespace {

[[nodiscard]] bool isAnyOf(const sol::object &, entt::type_list<>) {
  return false;
}
template <typename T, class... Rest>
[[nodiscard]] bool isAnyOf(const sol::object &obj,
                           entt::type_list<T, Rest...>) {
  return obj.is<T>() || isAnyOf(obj, entt::type_list<Rest...>{});
}

[[nodiscard]] auto makeGetter(const sol::table &table, const sol::object &key) {
  return [table, key](Variant &out) {
    const auto &obj = table[key];

    // clang-format off
#define TRY_GET(Type, convert)                                                 \
    if (obj.is<Type>()) { out = convert(obj.get<Type>()); }

    TRY_GET(bool, /**/)
    else TRY_GET(int, /**/)
    else TRY_GET(float, /**/)
    else TRY_GET(String, /**/)
    else TRY_GET(glm::vec2, to_Rml)
    else TRY_GET(glm::vec3, to_Rml)
    else TRY_GET(glm::vec4, to_Rml)

#undef TRY_GET
    // clang-format on
  };
}
[[nodiscard]] auto makeSetter(sol::table &table, const sol::object &key) {
  return [table, key](const Variant &in) mutable {
    switch (in.GetType()) {
#define CASE(Value, T, convert)                                                \
  case Variant::Type::Value:                                                   \
    table[key] = convert(in.Get<T>());                                         \
    break;

      CASE(BOOL, bool, /**/)
      CASE(INT, int, /**/)
      CASE(FLOAT, float, /**/)
      CASE(STRING, std::string, /**/)
      CASE(VECTOR2, Vector2f, to_glm)
      CASE(VECTOR3, Vector3f, to_glm)
      CASE(VECTOR4, Vector4f, to_glm)

#undef CASE
    }
  };
}

void bindMembers(DataModelConstructor &ctor, sol::table &table) {
  constexpr auto kBindableTypes =
    entt::type_list<bool, int, float, std::string, glm::vec2, glm::vec3,
                    glm::vec4>{};

  for (const auto &[key, value] : table) {
    const auto name = key.as<std::string>();

    if (value.is<sol::function>()) {
      ctor.BindEventCallback(
        name, [f = value.as<sol::function>()](DataModelHandle h, Event &evt,
                                              const VariantList &args) {
          f(h, evt, args);
        });
    } else if (isAnyOf(value, kBindableTypes)) {
      ctor.BindFunc(name, makeGetter(table, key), makeSetter(table, key));
    }
  }
}

} // namespace

ScriptDataModel::ScriptDataModel(Context &ctx, const String &name,
                                 sol::table &table)
    : m_constructor{ctx.CreateDataModel(name)} {
  bindMembers(m_constructor, table);
}
Rml::DataModelHandle ScriptDataModel::getModelHandle() const {
  return m_constructor.GetModelHandle();
}

void registerDataModel(sol::table &lua) {
  // clang-format off
  lua.DEFINE_USERTYPE(DataModelHandle,
    sol::no_constructor,

    "isVariableDirty", &DataModelHandle::IsVariableDirty,
    "dirtyVariable", &DataModelHandle::DirtyVariable,
    "dirtyAllVariables", &DataModelHandle::DirtyAllVariables,
    
    BIND_TOSTRING(DataModelHandle)
  );

  lua.DEFINE_USERTYPE(ScriptDataModel,
    "getModelHandle", &ScriptDataModel::getModelHandle,

    BIND_TOSTRING(ScriptDataModel)
  );
  // clang-format on
}
