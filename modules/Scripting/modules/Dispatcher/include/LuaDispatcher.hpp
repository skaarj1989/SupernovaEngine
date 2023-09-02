#pragma once

#include "entt/meta/factory.hpp"
#include "entt/signal/dispatcher.hpp"
#include "sol/table.hpp"

void registerDispatcher(sol::state &);

class LuaDispatcher {
public:
  LuaDispatcher() = delete;

  enum Functions : entt::hashed_string::size_type {
    // (entt::dispatcher *, const sol::function &) -> std::unique_ptr
    Connect = entt::hashed_string{"LuaDispatcher::_connect"}.value(),
    // (entt::dispatcher *, const sol::table &)
    Trigger = entt::hashed_string{"LuaDispatcher::_trigger"}.value(),
    // (entt::dispatcher *, const sol::table &)
    Enqueue = entt::hashed_string{"LuaDispatcher::_enqueue"}.value(),
    // (entt::dispatcher *)
    Clear = entt::hashed_string{"LuaDispatcher::_clear"}.value(),
    // (entt::dispatcher *)
    Update = entt::hashed_string{"LuaDispatcher::_update"}.value(),
  };

  template <typename Event> static void extendMetaType() {
    entt::meta<Event>()
      .template func<&LuaDispatcher::_connect<Event>>(Functions::Connect)
      .template func<&LuaDispatcher::_trigger<Event>>(Functions::Trigger)
      .template func<&LuaDispatcher::_enqueue<Event>>(Functions::Enqueue)
      .template func<&LuaDispatcher::_clear<Event>>(Functions::Clear)
      .template func<&LuaDispatcher::_update<Event>>(Functions::Update);
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
  template <typename Event>
  auto _connect(entt::dispatcher *dispatcher, const sol::function &f) {
    assert(dispatcher && f.valid());

    struct ScriptListener {
      ScriptListener(entt::dispatcher &dispatcher, const sol::function &f)
          : callback{f} {
        connection =
          dispatcher.sink<Event>().connect<&ScriptListener::receive>(*this);
      }
      ScriptListener(const ScriptListener &) = delete;
      ScriptListener(ScriptListener &&) noexcept = default;
      ~ScriptListener() {
        connection.release();
        callback.abandon();
      }

      ScriptListener &operator=(const ScriptListener &) = delete;
      ScriptListener &operator=(ScriptListener &&) noexcept = default;

      void receive(const Event &evt) {
        if (connection && callback.valid()) callback(evt);
      }

      sol::function callback;
      entt::connection connection;
    };

    return std::make_unique<ScriptListener>(*dispatcher, f);
  }

  template <typename Event>
  void _trigger(entt::dispatcher *dispatcher, const sol::table &evt) {
    assert(dispatcher && evt.valid());
    dispatcher->trigger(evt.as<Event>());
  }
  template <typename Event>
  void _enqueue(entt::dispatcher *dispatcher, const sol::table &evt) {
    assert(dispatcher && evt.valid());
    dispatcher->enqueue(evt.as<Event>());
  }
  template <typename Event> void _clear(entt::dispatcher *dispatcher) {
    assert(dispatcher);
    dispatcher->clear<Event>();
  }
  template <typename Event> void _update(entt::dispatcher *dispatcher) {
    assert(dispatcher);
    dispatcher->update<Event>();
  }
};
