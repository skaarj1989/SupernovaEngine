#include "ScriptSystem.hpp"
#include "ScriptContext.hpp"
#include "tracy/Tracy.hpp"
#include "spdlog/spdlog.h"

void ScriptSystem::setup(entt::registry &r, sol::state &lua) {
  r.ctx().emplace<ScriptContext>(lua);
  r.on_construct<ScriptComponent>().connect<&_initScriptComponent>();
  r.on_destroy<ScriptComponent>().connect<&_cleanupScriptComponent>();
}

void ScriptSystem::onInput(entt::registry &r, const os::InputEvent &evt) {
  ZoneScopedN("ScriptSystem::OnInput");
  for (auto [_, c] : r.view<ScriptComponent>().each()) {
    c.m_scriptNode.input(evt);
  }
}
void ScriptSystem::onUpdate(entt::registry &r, const float dt) {
  ZoneScopedN("ScriptSystem::Update");
  for (auto [_, c] : r.view<ScriptComponent>().each()) {
    c.m_scriptNode.update(dt);
  }
  {
    ZoneScopedN("Lua::CollectGarbage");
    getScriptContext(r).lua->collect_garbage();
  }
}
void ScriptSystem::onPhysicsStep(entt::registry &r, const float dt) {
  ZoneScopedN("ScriptSystem::OnPhysicsStep");
  for (auto [_, c] : r.view<ScriptComponent>().each()) {
    c.m_scriptNode.physicsStep(dt);
  }
}

//
// (private):
//

void ScriptSystem::_initScriptComponent(entt::registry &r,
                                        const entt::entity e) {
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
void ScriptSystem::_cleanupScriptComponent(entt::registry &r,
                                           const entt::entity e) {
  auto &c = r.get<ScriptComponent>(e);
  c.m_scriptNode.destroy();
}

//
// Helper:
//

ScriptContext &getScriptContext(entt::registry &r) {
  return r.ctx().get<ScriptContext>();
}
