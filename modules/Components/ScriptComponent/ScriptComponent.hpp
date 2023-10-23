#pragma once

#include "entt/core/type_info.hpp"
#include "ScriptManager.hpp"
#include "entt/entity/handle.hpp"
#include "ScriptNode.hpp"

class ScriptComponent {
  // Initializes m_scriptNode (when emplaced to a registry).
  friend class ScriptSystem;

  // Access to m_scriptNode.self inside a lua script.
  friend void registerScriptComponent(sol::state &);

public:
  explicit ScriptComponent(std::shared_ptr<ScriptResource> resource = {})
      : m_resource{resource} {}
  ScriptComponent(const ScriptComponent &other)
      : m_resource{other.m_resource} {}
  ScriptComponent(ScriptComponent &&) noexcept = default;
  ~ScriptComponent() = default;

  ScriptComponent &operator=(const ScriptComponent &) = delete;
  ScriptComponent &operator=(ScriptComponent &&) noexcept = default;

  [[nodiscard]] auto getResource() const { return m_resource; }

  // ---

  template <class Archive> void save(Archive &archive) const {
    archive(serialize(m_resource));
  }
  template <class Archive> void load(Archive &archive) {
    std::optional<std::string> path;
    archive(path);
    if (path) m_resource = loadResource<ScriptManager>(*path);
  }

private:
  std::shared_ptr<ScriptResource> m_resource;
  ScriptNode m_scriptNode;
};

static_assert(std::is_copy_constructible_v<ScriptComponent>);

template <> struct entt::type_hash<ScriptComponent> {
  [[nodiscard]] static constexpr entt::id_type value() noexcept {
    return 1680050497;
  }
};
