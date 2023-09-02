#pragma once

#include "ScriptContext.hpp"
#include "ScriptComponent.hpp"

#include "SystemCommons.hpp"

/*
  Context variables:
  - [creates] ScriptContext (state, environment)
  Components:
  - [setup callbacks] ScriptComponent
*/
struct ScriptSystem {
  INTRODUCE_COMPONENTS(ScriptComponent)

  static void setup(entt::registry &, sol::state &);

  static void onInput(entt::registry &, const os::InputEvent &);
  static void onUpdate(entt::registry &, float dt);
  static void onPhysicsStep(entt::registry &, float dt);

private:
  static void _initScriptComponent(entt::registry &, entt::entity);
  static void _cleanupScriptComponent(entt::registry &, entt::entity);
};

[[nodiscard]] ScriptContext &getScriptContext(entt::registry &);
