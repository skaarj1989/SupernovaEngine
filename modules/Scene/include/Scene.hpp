#pragma once

#include "NameComponent.hpp"
#include "Transform.hpp"
#include "HierarchySystem.hpp"
#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"
#include "AnimationSystem.hpp"
#include "AudioSystem.hpp"
#include "UISystem.hpp"
#include "ScriptSystem.hpp"

#include "entt/entity/handle.hpp"
#include "entt/entity/helper.hpp" // to_entity

class Scene {
public:
  // clang-format off
  static constexpr auto kComponentTypes =
    entt::type_list<NameComponent, Transform, ParentComponent, ChildrenComponent>{} +
    PhysicsSystem::kIntroducedComponents +
    RenderSystem::kIntroducedComponents +
    AnimationSystem::kIntroducedComponents +
    AudioSystem::kIntroducedComponents +
    UISystem::kIntroducedComponents +
    ScriptSystem::kIntroducedComponents;
  // clang-format on

  Scene();
  Scene(gfx::WorldRenderer &, RmlUiRenderInterface &, audio::Device &,
        sol::state &);
  Scene(const Scene &);
  Scene(Scene &&) noexcept = default;
  ~Scene();

  Scene &operator=(const Scene &);
  Scene &operator=(Scene &&) noexcept = default;

  [[nodiscard]] entt::registry &getRegistry();
  [[nodiscard]] const entt::registry &getRegistry() const;

  void copyFrom(const Scene &);

  entt::handle createEntity(std::optional<std::string> name = std::nullopt);
  [[nodiscard]] entt::handle get(entt::entity);
  entt::handle clone(entt::handle);

  template <class Component> auto getEntityId(Component &&c) const {
    return entt::to_entity(m_registry, std::forward<Component>(c));
  }

  auto each() { return m_registry.storage<entt::entity>().each(); }

  [[nodiscard]] PhysicsWorld *getPhysicsWorld();

  [[nodiscard]] bool empty() const;
  void clear();

  // -- Serialization:

  enum class ArchiveType { Binary = 0, JSON = 1 };

  bool save(const std::filesystem::path &, const ArchiveType) const;
  bool load(const std::filesystem::path &, const ArchiveType);

private:
  entt::registry m_registry;
};
