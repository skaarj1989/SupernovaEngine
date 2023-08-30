#pragma once

#include "sol/table.hpp"
#include "os/InputEvents.hpp"

class ScriptNode {
public:
  ScriptNode() = default;
  explicit ScriptNode(sol::table &&);
  ~ScriptNode() = default;

  const sol::table &self() const;

  void init();
  void input(const os::InputEvent &evt);
  void update(float);
  void physicsStep(float);
  void destroy();

private:
  template <typename... Args>
  void _call(sol::protected_function &f, Args &&...args) {
    if (m_state == State::Ready && f) {
      f(m_self, std::forward<Args>(args)...);
    }
  }

private:
  enum class State { Invalid = -1, Uninitialized, Ready };
  State m_state{State::Uninitialized};

  sol::table m_self;

  struct Hooks {
    sol::protected_function init;
    sol::protected_function input;
    sol::protected_function update;
    sol::protected_function physicsStep;
    sol::protected_function destroy;
  };
  Hooks m_hooks{};
};
