#pragma once

#include "NameComponent.hpp"
#include "ParentComponent.hpp"
#include "ChildrenComponent.hpp"
#include "Transform.hpp"

#include "physics/PhysicsWorld.hpp"
#include "physics/Collider.hpp"
#include "physics/RigidBody.hpp"
#include "physics/Character.hpp"

#include "renderer/Light.hpp"
#include "renderer/MeshInstance.hpp"
#include "renderer/DecalInstance.hpp"
#include "CameraComponent.hpp"

#include "animation/SkeletonComponent.hpp"
#include "animation/AnimationComponent.hpp"
#include "animation/PlaybackController.hpp"

#include "ScriptComponent.hpp"

#include "entt/signal/emitter.hpp"
#include "entt/entity/registry.hpp"
#include "entt/entity/handle.hpp"
#include "entt/entity/helper.hpp" // to_entity

void copyRegistry(entt::registry &src, entt::registry &dst);

class Scene {
public:
  // clang-format off
  static constexpr entt::type_list<
    NameComponent,
    Transform,
    ParentComponent, ChildrenComponent,
    ColliderComponent, RigidBody, Character,
    CameraComponent,
    gfx::Light, gfx::MeshInstance, gfx::DecalInstance,
    SkeletonComponent, AnimationComponent, PlaybackController,
    ScriptComponent>
  kComponentTypes{};
  // clang-format on

  Scene();
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

  void clear();

  // -- Serialization:

  bool save(const std::filesystem::path &) const;
  bool load(const std::filesystem::path &);

private:
  entt::registry m_registry;
};
