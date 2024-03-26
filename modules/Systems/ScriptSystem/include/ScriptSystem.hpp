#pragma once

#include "ScriptComponent.hpp"
#include "SystemCommons.hpp"

struct ScriptContext;

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
  static void onUpdate(entt::registry &, const float dt);
  static void onPhysicsStep(entt::registry &, const float dt);

private:
  static void _initScriptComponent(entt::registry &, const entt::entity);
  static void _cleanupScriptComponent(entt::registry &, const entt::entity);
};

[[nodiscard]] ScriptContext &getScriptContext(entt::registry &);
