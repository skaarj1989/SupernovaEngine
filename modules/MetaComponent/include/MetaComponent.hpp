#pragma once

#include "entt/entity/registry.hpp"
#include "entt/entity/handle.hpp"
#include "entt/meta/factory.hpp"

/* @example
  constexpr entt::type_list<Foo, Bar, Baz> kTypes;
  MetaComponent::registerMetaComponents(kTypes);

  MetaComponent::registerMetaComponents2<Foo, Bar, Baz>();
*/

// Create meta_type for given component(s).
class MetaComponent {
public:
  MetaComponent() = delete;

  enum Functions : entt::hashed_string::size_type {
    // (entt::handle) -> bool
    Has = entt::hashed_string{"entt::handle::any_of"}.value(),
    // (entt::handle, T &component) -> T &
    Emplace = entt::hashed_string{"entt::handle::emplace"}.value(),
    // (entt::handle) -> T &
    Get = entt::hashed_string{"entt::handle::get"}.value(),
    // (entt::handle) -> entt::handle::size_t
    Remove = entt::hashed_string{"entt::handle::remove"}.value(),
  };

  template <class T> static void registerMetaComponent() {
    entt::meta<T>()
      .template func<&entt::handle::any_of<T>>(Functions::Has)
      .template func<&entt::handle::emplace<T>, entt::as_ref_t>(
        Functions::Emplace)
      .template func<&entt::handle::get<T>, entt::as_ref_t>(Functions::Get)
      .template func<&entt::handle::remove<T>>(Functions::Remove);
  }
  static void registerMetaComponents(entt::type_list<>) {}

  template <class T, class... Rest>
  static void registerMetaComponents(entt::type_list<T, Rest...>) {
    registerMetaComponent<T>();
    registerMetaComponents(entt::type_list<Rest...>());
  }

  template <class... Ts> static void registerMetaComponents2() {
    registerMetaComponents(entt::type_list<Ts...>());
  }
};
