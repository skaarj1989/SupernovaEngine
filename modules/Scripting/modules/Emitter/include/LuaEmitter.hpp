#pragma once

#include "entt/meta/factory.hpp"
#include "MetaHelper.hpp"
#include "ScriptTypeInfo.hpp"
#include "math/Hash.hpp"

/*
emitter:connect(SomeEvent, function(evt, sender)
  -- ...
end)
emitter:disconnect(SomeEvent)
*/

// Create meta_type of a given Event
// and extend it for an Emitter (anything derived from entt::emitter).
class LuaEmitter {
  enum Functions : entt::hashed_string::size_type {
    // (Emitter *, sol::function) -> void
    Connect = entt::hashed_string{"LuaEmitter::_connect"}.value(),
    // (Emitter *) -> void
    Disconnect = entt::hashed_string{"LuaEmitter::_disconnect"}.value(),
  };

public:
  LuaEmitter() = delete;

  template <class Emitter, typename Event> static void createMetaEvent() {
    entt::meta<Event>()
      .template func<&_connect<Emitter, Event>>(
        _hash<Emitter>(Functions::Connect))
      .template func<&_disconnect<Emitter, Event>>(
        _hash<Emitter>(Functions::Disconnect));
  }

  template <class Emitter> [[nodiscard]] static auto makeConnectLambda() {
    return [](Emitter &self, const sol::object &typeOrId,
              const sol::function &listener) {
      if (listener.valid()) {
        if (const auto eventId = getTypeId(typeOrId); eventId) {
          invokeMetaFunc(*eventId, _hash<Emitter>(Functions::Connect), &self,
                         listener);
        }
      }
    };
  }
  template <class Emitter> [[nodiscard]] static auto makeDisconnectLambda() {
    return [](Emitter &self, const sol::object &typeOrId) {
      if (const auto eventId = getTypeId(typeOrId); eventId) {
        invokeMetaFunc(*eventId, _hash<Emitter>(Functions::Disconnect), &self);
      }
    };
  }

private:
  template <class Emitter, typename Event>
  static void _connect(Emitter *emitter, const sol::function &f) {
    assert(emitter);
    emitter->template on<Event>(
      [f](const Event &evt, Emitter &sender) { f(evt, sender); });
  }

  template <class Emitter, typename Event>
  static void _disconnect(Emitter *emitter) {
    assert(emitter);
    emitter->template erase<Event>();
  }

  template <class Emitter>
  [[nodiscard]] static auto _hash(std::size_t functionId) {
    hashCombine(functionId, typeid(Emitter).hash_code());
    return static_cast<entt::id_type>(functionId);
  }
};
