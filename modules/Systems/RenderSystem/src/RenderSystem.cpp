#include "RenderSystem.hpp"
#include "rhi/RenderDevice.hpp"
#include "renderer/WorldRenderer.hpp"
#include "renderer/WorldView.hpp"
#include "Transform.hpp"
#include <format>

namespace {

template <typename T>
  requires std::is_base_of_v<gfx::MeshInstance, T>
void connectMeshInstance(entt::registry &r) {
  static const auto setUserData = [](entt::registry &r, const entt::entity e) {
    r.get<T>(e).setUserData(entt::to_integral(e));
  };
  r.on_construct<T>().connect<setUserData>();
  r.on_update<T>().connect<setUserData>();
}

void initCamera(entt::registry &r, const entt::entity e) {
  if (auto &skyLight = r.get<CameraComponent>(e).skyLight; skyLight.source) {
    assert(!skyLight.diffuse && !skyLight.specular);
    skyLight = getRenderer(r).createSkyLight(skyLight.source);
  }
}

[[nodiscard]] auto getLights(entt::registry &r, DebugDraw *debugDraw) {
  ZoneScopedN("GetLights");

  const auto view = r.view<const Transform, gfx::Light>();

  std::vector<const gfx::Light *> lights;
  lights.reserve(view.size_hint());

  for (auto [_, transform, light] : view.each()) {
    if (light.visible) {
      light.position = transform.getPosition();
      light.direction = transform.getForward();
      lights.emplace_back(&light);
    }

    if (debugDraw && light.debugVolume) {
      switch (light.type) {
      case gfx::LightType::Point:
        debugDraw->addSphere(light.range, light.color,
                             transform.getWorldMatrix());
        break;
      case gfx::LightType::Spot:
        debugDraw->addFrustum(
          glm::inverse(buildSpotLightMatrix(light).viewProjection()),
          light.color);
        break;

      default:
        break;
      }
    }
  }
  return lights;
}
template <class T>
[[nodiscard]] auto getMeshes(entt::registry &r, DebugDraw *debugDraw) {
  ZoneScopedN("GetMeshes");

  const auto view = r.view<const Transform, T>();

  std::vector<const T *> meshes;
  meshes.reserve(view.size_hint());

  for (auto [_, transform, meshInstance] : view.each()) {
    meshInstance.setTransform(transform);
    meshes.emplace_back(&meshInstance);

    if (debugDraw) {
      for (const auto &sm : meshInstance.each()) {
        if (bool(sm.flags & gfx::SubMeshInstance::Flags::ShowAABB))
          debugDraw->addAABB(sm.aabb);
      }
    }
  }
  return meshes;
}

[[nodiscard]] auto requiresNewRenderTarget(const CameraComponent &cc) {
  return cc.extent && (!cc.target || cc.target->getExtent() != cc.extent);
}
[[nodiscard]] auto createRenderTarget(rhi::RenderDevice &rd,
                                      const rhi::Extent2D extent) {
  using enum rhi::ImageUsage;
  return rhi::Texture::Builder{}
    .setExtent(extent)
    .setPixelFormat(rhi::PixelFormat::BGRA8_UNorm)
    .setNumMipLevels(1)
    .setNumLayers(std::nullopt)
    .setUsageFlags(Transfer /* For blit. */ | RenderTarget | Sampled)
    .setupOptimalSampler(true)
    .build(rd);
}

} // namespace

void RenderSystem::setup(entt::registry &r, gfx::WorldRenderer &wr) {
  auto &ctx = r.ctx();
  ctx.emplace<gfx::WorldRenderer *>(&wr);
  ctx.emplace<AABB>(AABB{
    .min = glm::vec3{-10.0f},
    .max = glm::vec3{10.0f},
  });
  ctx.emplace<MainCamera>();

  connectMeshInstance<gfx::MeshInstance>(r);
  connectMeshInstance<gfx::DecalInstance>(r);

  r.on_construct<CameraComponent>().connect<&initCamera>();
}

void RenderSystem::update(entt::registry &r, rhi::CommandBuffer &cb,
                          const float dt, const gfx::SceneView *mainSceneView,
                          gfx::DebugOutput *debugOutput) {
  ZoneScopedN("RenderSystem::Update");

  const auto view = r.view<const Transform, CameraComponent>();
  const auto numSceneViews = mainSceneView ? 1 : 0 + view.size_hint();
  if (numSceneViews == 0) return;

  auto &renderer = getRenderer(r);
  auto &renderDevice = renderer.getRenderDevice();

  DebugDraw *debugDraw{nullptr};

  std::vector<gfx::SceneView> sceneViews;
  sceneViews.reserve(numSceneViews);
  if (mainSceneView) {
    debugDraw = mainSceneView->debugDraw;
    sceneViews.emplace_back(*mainSceneView);
  }

  auto lights = getLights(r, debugDraw);
  auto meshes = getMeshes<gfx::MeshInstance>(r, debugDraw);
  auto decals = getMeshes<gfx::DecalInstance>(r, debugDraw);

  for (auto [e, transform, cameraComponent] : view.each()) {
    auto &camera = cameraComponent.camera;
    camera.fromTransform(transform);

    auto &target = cameraComponent.target;
    if (requiresNewRenderTarget(cameraComponent)) {
      target = rhi::makeShared<rhi::Texture>(
        renderDevice, createRenderTarget(renderDevice, cameraComponent.extent));
    }
    if (target) {
      sceneViews.emplace_back(
        createSceneView(std::format("entity: {}", entt::to_integral(e)),
                        cameraComponent, &camera, target.get()));
    }
  }

  renderer.drawFrame(cb,
                     {
                       .aabb = r.ctx().get<AABB>(),
                       .lights = lights,
                       .meshes = meshes,
                       .decals = decals,
                       .sceneViews = sceneViews,
                     },
                     dt, debugOutput);
}

//
// Helper:
//

gfx::SceneView createSceneView(std::string name, CameraComponent &c,
                               const gfx::PerspectiveCamera *overrideCamera,
                               rhi::Texture *overrideTarget) {
  return {
    .name = std::move(name),
    .target = overrideTarget ? *overrideTarget : *c.target,
    .camera = overrideCamera ? *overrideCamera : c.camera,
    .renderSettings = c.renderSettings,
    .skyLight = c.skyLight ? &c.skyLight : nullptr,
    .postProcessEffects = c.postProcessEffects,
    .debugDraw = &c.debugDraw,
  };
}

gfx::WorldRenderer &getRenderer(entt::registry &r) {
  return *r.ctx().get<gfx::WorldRenderer *>();
}

MainCamera &getMainCamera(entt::registry &r) {
  return r.ctx().get<MainCamera>();
}
CameraComponent *getMainCameraComponent(entt::registry &r) {
  return r.try_get<CameraComponent>(getMainCamera(r).e);
}
