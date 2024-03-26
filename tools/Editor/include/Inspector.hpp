#pragma once

#include "MetaComponent.hpp"
#include "MetaHelper.hpp"
#include "entt/entity/handle.hpp"

/* @example
  class Foo : public Inspector<Foo> {
    // overload _onInspect(entt::handle, ComponentType &)
  };

  // Option 1.
  constexpr entt::type_list<Type1, Type2> types;
  _connectInspectors(types);

  // Option 2:
  _connectInspectors2<Type1, Type2>();
*/

template <class Derived> class Inspector {
public:
  enum Functions : entt::hashed_string::size_type {
    // (App *, entt::handle, void *component)
    Inspect = entt::hashed_string{"Inspector::_onInspect"}.value(),
  };

protected:
  template <class Component> static void _connectInspector() {
    entt::meta<Component>()
      .template func<&Inspector<Derived>::_onInspect<Component>>(
        Functions::Inspect);
  }
  template <class Component, typename... Rest>
  static void _connectInspectors(entt::type_list<Component, Rest...>) {
    _connectInspector<Component>();
    if constexpr (auto typeList = entt::type_list<Rest...>{};
                  typeList.size > 0) {
      _connectInspectors(typeList);
    }
  }

  template <class... Components> static void _connectInspectors2() {
    _connectInspectors(entt::type_list<Components...>{});
  }

  // ---

  bool _onGui(const entt::id_type componentId, const entt::handle h) {
    assert(h);
    auto metaType = entt::resolve(componentId);
    return metaType ? _onGui(metaType, h) : false;
  }
  bool _onGui(entt::meta_type metaType, const entt::handle h) {
    assert(h);
    auto component = invokeMetaFunc(metaType, MetaComponent::Functions::Get, h);
    return _onGui(metaType, h, component.data());
  }

private:
  bool _onGui(entt::meta_type metaType, const entt::handle h, void *component) {
    assert(component); // MetaComponent::registerMetaComponents
    if (auto metaFunction = metaType.func(Functions::Inspect); metaFunction) {
      metaFunction.invoke({}, static_cast<Derived *>(this), h, component);
      return true;
    }
    return false;
  }

private:
  template <class Component>
  static void _onInspect(Derived *app, const entt::handle h, void *component) {
    assert(app && h && component);
    app->_onInspect(h, *static_cast<Component *>(component));
  }
};
