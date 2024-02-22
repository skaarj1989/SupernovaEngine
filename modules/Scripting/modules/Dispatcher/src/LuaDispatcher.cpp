#include "LuaDispatcher.hpp"
#include "sol/state.hpp"

#include "ScriptTypeInfo.hpp"
#include "MetaHelper.hpp"
#include "Sol2HelperMacros.hpp"

namespace {

void registerEventInheritance(sol::state &lua) {
  lua.do_string(
    R"(
function define_event()
    assert(BaseScriptEvent ~= nil)

    local ScriptEvent = {}
    ScriptEvent.__index = ScriptEvent

    function ScriptEvent:_init(evt)
        for k, v in pairs(evt) do
            self[k] = v
        end
    end
    setmetatable(ScriptEvent, {
        __index = BaseScriptEvent, -- This is what makes the inheritance work
        __call = function(cls, ...)
            local self = setmetatable({}, cls)
            self:_init(... or {})
            return self
        end
    })
    assert(ScriptEvent.type_id() == BaseScriptEvent.type_id())
    return ScriptEvent
end
)");
}

} // namespace

void registerDispatcher(sol::state &lua) {
  // Used for events defined in lua scripts
  struct BaseScriptEvent {
    sol::table self;
  };
  // clang-format off
  lua.DEFINE_USERTYPE(BaseScriptEvent,
    BIND_TYPEID(BaseScriptEvent),
    BIND_TOSTRING(BaseScriptEvent)
  );
  // clang-format on
  registerEventInheritance(lua);

  struct ScriptedEventListener final {
    ScriptedEventListener(entt::dispatcher &dispatcher, const sol::table &type,
                          const sol::function &f)
        : key{getKey(type)}, callback{f} {
      connection = dispatcher.sink<BaseScriptEvent>()
                     .connect<&ScriptedEventListener::receive>(*this);
    }
    ScriptedEventListener(const ScriptedEventListener &) = delete;
    ScriptedEventListener(ScriptedEventListener &&) noexcept = default;
    ~ScriptedEventListener() { connection.release(); }

    ScriptedEventListener &operator=(const ScriptedEventListener &) = delete;
    ScriptedEventListener &
    operator=(ScriptedEventListener &&) noexcept = default;

    static uintptr_t getKey(const sol::table &t) {
      return std::bit_cast<uintptr_t>(t["__index"].get<sol::table>().pointer());
    }

    void receive(const BaseScriptEvent &evt) const {
      assert(connection && callback.valid());
      if (auto &[self] = evt; getKey(self) == key) callback(self);
    }

    const uintptr_t key;
    const sol::function callback;
    entt::connection connection;
  };

  using Dispatcher = entt::dispatcher;

  // clang-format off
  lua.DEFINE_USERTYPE(Dispatcher,
    sol::call_constructor,
    sol::constructors<Dispatcher()>(),

    "trigger",
      [](entt::dispatcher &self, const sol::table &evt) {
        if (const auto eventId = getTypeId(evt); eventId) {
          if (*eventId == entt::type_hash<BaseScriptEvent>::value()) {
            // Event defined in a script.
            self.trigger(BaseScriptEvent{evt});
          }
          else {
            // Native event (registered via 'LuaDispatcher::extendMetaType').
            invokeMetaFunc(*eventId, LuaDispatcher::Functions::Trigger, &self, evt);
          }
        }
      },
    "enqueue",
      [](entt::dispatcher &self, const sol::table &evt) {
        if (const auto eventId = getTypeId(evt); eventId) {
          if (*eventId == entt::type_hash<BaseScriptEvent>::value()) {
            self.enqueue(BaseScriptEvent{evt});
          }
          else {
            invokeMetaFunc(*eventId, LuaDispatcher::Functions::Trigger, &self, evt);
          }
        }
      },
    "clear",
      sol::overload(
        [](entt::dispatcher &self) { self.clear(); },
        [](entt::dispatcher &self, const sol::object &type) {
          if (auto typeId = getTypeId(type); typeId) {
            invokeMetaFunc(*typeId, LuaDispatcher::Functions::Clear, &self);
          }
        }
      ),
    "update",
      sol::overload(
        [](const entt::dispatcher &self) { self.update(); },
        [](entt::dispatcher &self, const sol::object &type) {
          if (auto typeId = getTypeId(type); typeId) {
            invokeMetaFunc(*typeId, LuaDispatcher::Functions::Update, &self);
          }
        }
      ),
    "connect",
      [](entt::dispatcher &self, const sol::object &type,
         const sol::function &listener, sol::this_state) {
        if (!listener.valid()) {
          return entt::meta_any{};
        }
        if (auto eventId = getTypeId(type); eventId) {
          if (*eventId == entt::type_hash<BaseScriptEvent>::value()) {
            return entt::meta_any{
              std::make_unique<ScriptedEventListener>(self, type, listener)};
          }
          else {
            return invokeMetaFunc(*eventId, LuaDispatcher::Functions::Connect, &self, listener);
          }
        }
        return entt::meta_any{};
      },
    "disconnect",
      [](const entt::dispatcher &, const sol::table &connection) {
        connection.as<entt::meta_any>().reset();
      },

    BIND_TOSTRING(Dispatcher)
  );
  // clang-format on
}
