#pragma once

#include "entt/meta/factory.hpp"
#include "sol/function.hpp"

/*
function callback(evt, sender)
  -- ...
end

emitter:connect(SomeEvent, callback)
emitter:disconnect(SomeEvent)
*/

// Create meta_type of a given Event
// and extend it for an Emitter (anything derived from entt::emitter).
class LuaEmitter {
public:
  LuaEmitter() = delete;

  enum Functions : entt::hashed_string::size_type {
    // (Emitter *, sol::function) -> void
    Connect = entt::hashed_string{"LuaEmitter::_connect"}.value(),
    // (Emitter *) -> void
    Disconnect = entt::hashed_string{"LuaEmitter::_disconnect"}.value(),
  };

  template <class Emitter, typename Event> static void createMetaEvent() {
    entt::meta<Event>()
      .template func<&LuaEmitter::_connect<Emitter, Event>>(Functions::Connect)
      .template func<&LuaEmitter::_disconnect<Emitter, Event>>(
        Functions::Disconnect);
  }

private:
  template <typename Emitter, typename Event>
  static void _connect(Emitter *emitter, const sol::function &f) {
    assert(emitter);
    emitter->on<Event>(
      [f](const Event &evt, Emitter &sender) { f(evt, sender); });
  }

  template <typename Emitter, typename Event>
  static void _disconnect(Emitter *emitter) {
    assert(emitter);
    emitter->erase<Event>();
  }
};
