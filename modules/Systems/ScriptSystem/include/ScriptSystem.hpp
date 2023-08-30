#pragma once

#include "SystemCommons.hpp"

#include "ScriptContext.hpp"
#include "ScriptComponent.hpp"

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

  template <class Archive> static void save(Archive &archive) {
    auto &[_, snapshot] = cereal::get_user_data<OutputContext>(archive);
    snapshot.get<ScriptComponent>(archive);
  }
  template <class Archive> static void load(Archive &archive) {
    auto &[_, snapshotLoader] = cereal::get_user_data<InputContext>(archive);
    snapshotLoader.get<ScriptComponent>(archive);
  }

private:
  static void _initScriptComponent(entt::registry &, entt::entity);
  static void _cleanupScriptComponent(entt::registry &, entt::entity);
};

[[nodiscard]] ScriptContext &getScriptContext(entt::registry &);
