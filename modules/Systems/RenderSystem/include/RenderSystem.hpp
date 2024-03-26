#pragma once

#include "renderer/SceneView.hpp"
#include "renderer/Light.hpp"
#include "renderer/DecalInstance.hpp"
#include "CameraComponent.hpp"

#include "SystemCommons.hpp"

struct MainCamera {
  entt::entity e{entt::null};
};

namespace gfx {
class WorldRenderer;
struct DebugOutput;
} // namespace gfx

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

  static void update(entt::registry &r, rhi::CommandBuffer &, const float dt,
                     const gfx::SceneView *mainSceneView = nullptr,
                     gfx::DebugOutput * = nullptr);

  template <class Archive> static void save(Archive &archive) {
    auto &[registry, _] = cereal::get_user_data<OutputContext>(archive);
    auto &ctx = registry.ctx();
    archive(ctx.template get<AABB>());
    archive(ctx.template get<MainCamera>().e);
  }
  template <class Archive> static void load(Archive &archive) {
    auto &[registry, _] = cereal::get_user_data<InputContext>(archive);
    auto &ctx = registry.ctx();
    archive(ctx.template get<AABB>());
    archive(ctx.template get<MainCamera>().e);
  }
};

[[nodiscard]] gfx::SceneView
createSceneView(std::string, CameraComponent &,
                const gfx::PerspectiveCamera *overrideCamera,
                rhi::Texture *overrideTarget);

[[nodiscard]] gfx::WorldRenderer &getRenderer(entt::registry &);
[[nodiscard]] MainCamera &getMainCamera(entt::registry &);
[[nodiscard]] CameraComponent *getMainCameraComponent(entt::registry &);
