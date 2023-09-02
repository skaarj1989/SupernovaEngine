#pragma once

#include "entt/entity/registry.hpp"
#include "entt/entity/handle.hpp"
#include "entt/meta/factory.hpp"
#include "sol/table.hpp"

// register entt::handle user type in lua
void registerEntityHandle(sol::state &);

// Extends meta_type of a given component(s)
// with functions to get/emplace from a script
class LuaComponent {
public:
  LuaComponent() = delete;

  enum Functions : entt::hashed_string::size_type {
    // (entt::handle, const sol::table &, sol::this_state) -> sol::reference
    Emplace = entt::hashed_string{"LuaComponent::_emplace"}.value(),
    // (entt::handle, sol::this_state) -> sol::reference (may be nil)
    Get = entt::hashed_string{"LuaComponent::_get"}.value(),
  };

  template <class T> static void extendMetaType() {
    entt::meta<T>()
      .template func<&LuaComponent::_emplace<T>>(Functions::Emplace)
      .template func<&LuaComponent::_get<T>>(Functions::Get);
  }
  template <class T, class... Rest>
  static void extendMetaTypes(entt::type_list<T, Rest...>) {
    extendMetaType<T>();
    if constexpr (auto typeList = entt::type_list<Rest...>{};
                  typeList.size > 0) {
      extendMetaTypes(typeList);
    }
  }

  template <class... Ts> static void extendMetaTypes2() {
    extendMetaTypes(entt::type_list<Ts...>{});
  }

private:
  template <class T>
  static auto _emplace(entt::handle h, sol::table &instance,
                       const sol::this_state s) {
    // Guaranteed in registerEntityHandle -> "emplace"
    assert(instance.valid());

    // In case that a type is passed (instead of an object):
    // self.entity:emplace(ComponentType)
    // 1. instance.is<T>() -> false
    // 2. instance.get_type() -> sol::type::table

    h.remove<T>();
    auto &comp =
      h.emplace<T>(instance.is<T>() ? std::move(instance.as<T &&>()) : T{});

    return sol::make_reference(s, std::ref(comp));
  }
  template <class T> static auto _get(entt::handle h, const sol::this_state s) {
    auto *comp = h.try_get<T>();
    return comp ? sol::make_reference(s, comp) : sol::lua_nil_t{};
  }
};
