#include "ScriptSystem.hpp"
#include "spdlog/spdlog.h"

void ScriptSystem::setup(entt::registry &r, sol::state &lua) {
  auto &scriptContext = r.ctx().emplace<ScriptContext>(lua);
  auto &defaultEnv = scriptContext.defaultEnv;
  defaultEnv["createEntity"] = [&r] { return entt::handle{r, r.create()}; };
  defaultEnv["getEntity"] = [&r](entt::entity e) { return entt::handle{r, e}; };

  r.on_construct<ScriptComponent>().connect<&_initScriptComponent>();
  r.on_destroy<ScriptComponent>().connect<&_cleanupScriptComponent>();
}

void ScriptSystem::onInput(entt::registry &r, const os::InputEvent &evt) {
  for (auto [_, c] : r.view<ScriptComponent>().each()) {
    c.m_scriptNode.input(evt);
  }
}
void ScriptSystem::onUpdate(entt::registry &r, float dt) {
  for (auto [_, c] : r.view<ScriptComponent>().each()) {
    c.m_scriptNode.update(dt);
  }

  getScriptContext(r).lua->collect_garbage();
}
void ScriptSystem::onPhysicsStep(entt::registry &r, float dt) {
  for (auto [_, c] : r.view<ScriptComponent>().each()) {
    c.m_scriptNode.physicsStep(dt);
  }
}

//
// (private):
//

void ScriptSystem::_initScriptComponent(entt::registry &r, entt::entity e) {
  auto &c = r.get<ScriptComponent>(e);
  if (!c.m_resource) return;

  const auto &[lua, defaultEnv] = getScriptContext(r);
  auto result = lua->script(c.m_resource->code, defaultEnv);
  if (!result.valid()) {
    SPDLOG_ERROR("Invalid lua script.");
    c.m_resource = {};
    return;
  }
  if (result.get_type() != sol::type::table) {
    SPDLOG_ERROR("Invalid lua script, expected table (ScriptNode).");
    c.m_resource = {};
    return;
  }

  auto self = result.get<sol::table>();
  self["entity"] = entt::handle{r, e};
  c.m_scriptNode = ScriptNode{std::move(self)};
}
void ScriptSystem::_cleanupScriptComponent(entt::registry &r, entt::entity e) {
  auto &c = r.get<ScriptComponent>(e);
  c.m_scriptNode.destroy();
}

//
// Helper:
//

ScriptContext &getScriptContext(entt::registry &r) {
  return r.ctx().get<ScriptContext>();
}
