#include "ScriptNode.hpp"
#include "spdlog/spdlog.h"

ScriptNode::ScriptNode(sol::table &&t) : m_self{std::move(t)} {
  const auto errorHandler = [this](const char *msg) {
    SPDLOG_ERROR(msg);
    m_state = State::Invalid;
  };

#define GET_FUNCTION(Name)                                                     \
  .Name = sol::protected_function{m_self[#Name], errorHandler}

  // clang-format off
  m_hooks = {
    GET_FUNCTION(init),
    GET_FUNCTION(input),   
    GET_FUNCTION(update),
    GET_FUNCTION(physicsStep), 
    GET_FUNCTION(destroy),
  };
  // clang-format on

#undef GET_FUNCTION
}

const sol::table &ScriptNode::self() const { return m_self; }

void ScriptNode::init() {
  m_state = State::Ready;
  _call(m_hooks.init);
}
void ScriptNode::input(const os::InputEvent &evt) { _call(m_hooks.input, evt); }
void ScriptNode::update(float dt) {
  if (m_state == State::Invalid) return;
  if (m_state == State::Uninitialized) init();
  _call(m_hooks.update, dt);
}
void ScriptNode::physicsStep(float dt) { _call(m_hooks.physicsStep, dt); }
void ScriptNode::destroy() { _call(m_hooks.destroy); }
