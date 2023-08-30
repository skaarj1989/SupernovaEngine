#include "LuaScheduler.hpp"
#include "sol/state.hpp"
#include "sol/variadic_args.hpp"
#include "entt/process/scheduler.hpp"
#include "Sol2HelperMacros.hpp"

#include <chrono>

using namespace std::chrono_literals;

using fsec = std::chrono::duration<float>;

class ScriptProcess final : public entt::process<ScriptProcess, fsec> {
public:
  ScriptProcess(const sol::table &t, fsec freq = 250ms)
      : m_self{t}, m_update{m_self["update"]}, m_frequency{freq} {
#define BIND(func) m_self.set_function(#func, &ScriptProcess::##func, this)

    BIND(succeed);
    BIND(fail);
    BIND(pause);
    BIND(unpause);

    BIND(abort);
    BIND(alive);
    BIND(finished);
    BIND(paused);
    BIND(rejected);

#undef BIND
  }

  void init() { _call("init"); }

  void update(fsec dt, void *) {
    if (!m_update.valid()) return fail();

    m_time += dt;
    if (m_time >= m_frequency) {
      m_update(m_self, dt.count());
      m_time = 0s;
    }
  }
  void succeeded() { _call("succeeded"); }
  void failed() { _call("failed"); }
  void aborted() { _call("aborted"); }

private:
  void _call(const std::string_view function_name) {
    if (auto &&f = m_self[function_name]; f.valid()) f(m_self);
  }

private:
  sol::table m_self;
  sol::protected_function m_update;

  fsec m_frequency;
  fsec m_time{0};
};

void registerScheduler(sol::state &lua) {
  using Scheduler = entt::basic_scheduler<fsec>;

#define BIND(Member) _BIND(Scheduler, Member)
  // clang-format off
  DEFINE_USERTYPE(Scheduler,
    sol::call_constructor,
    sol::constructors<Scheduler()>(),

    BIND(size),
    BIND(empty),
    BIND(clear),
    
    "attach",
      [](Scheduler &self, const sol::table &process,
         const sol::variadic_args &va) {
        auto continuator =
          self.attach<ScriptProcess>(process);
        for (auto &&childProcess : va) {
          continuator =
            continuator.then<ScriptProcess>(childProcess);
        }
      },

    "update", [](Scheduler &self, fsec dt) { self.update(dt); },
    "abort", [](Scheduler &self) { self.abort(); },

    BIND_TOSTRING(Scheduler)
  );
  // clang-format on
#undef BIND
}
