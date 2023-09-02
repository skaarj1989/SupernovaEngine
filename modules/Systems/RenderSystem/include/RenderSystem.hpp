#pragma once

#include "renderer/WorldRenderer.hpp"
#include "CameraComponent.hpp"

#include "SystemCommons.hpp"

struct MainCamera {
  entt::entity e{entt::null};
};

/*
  Context variables:
  - [creates] AABB (world bounds, used for Global Illumination)
  - [creates] MainCamera
  - [creates] gfx::WorldRenerer *
  Components:
  - [setup callbacks] CameraComponent
  - Transform
  - MeshInstance
  - DecalInstance
*/
class RenderSystem {
public:
  INTRODUCE_COMPONENTS(CameraComponent, gfx::Light, gfx::MeshInstance,
                       gfx::DecalInstance)

  static void setup(entt::registry &r, gfx::WorldRenderer &);

  static void update(entt::registry &r, rhi::CommandBuffer &, float dt,
                     const gfx::SceneView *mainSceneView = nullptr,
                     gfx::DebugOutput * = nullptr);

  template <class Archive> static void save(Archive &archive) {
    auto &[registry, _] = cereal::get_user_data<OutputContext>(archive);
    auto &ctx = registry.ctx();
    archive(ctx.get<AABB>());
    archive(ctx.get<MainCamera>().e);
  }
  template <class Archive> static void load(Archive &archive) {
    auto &[registry, _] = cereal::get_user_data<InputContext>(archive);
    auto &ctx = registry.ctx();
    archive(ctx.get<AABB>());
    archive(ctx.get<MainCamera>().e);
  }
};

[[nodiscard]] gfx::SceneView
createSceneView(std::string, CameraComponent &,
                const gfx::PerspectiveCamera *overrideCamera,
                rhi::Texture *overrideTarget);

[[nodiscard]] gfx::WorldRenderer &getRenderer(entt::registry &);
[[nodiscard]] MainCamera &getMainCamera(entt::registry &);
[[nodiscard]] CameraComponent *getMainCameraComponent(entt::registry &);
