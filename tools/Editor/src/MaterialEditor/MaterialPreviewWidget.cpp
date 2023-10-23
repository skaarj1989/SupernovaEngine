#include "MaterialEditor/MaterialPreviewWidget.hpp"
#include "Services.hpp"

#include "IconsFontAwesome6.h"
#include "ImGuiHelper.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "imgui_internal.h"

#include "Inspectors/TransformInspector.hpp"
#include "Inspectors/LightInspector.hpp"
#include "Inspectors/MaterialInstanceInspector.hpp"

#include "RenderSettings.hpp"
#include "CameraController.hpp"
#include "SceneEditor/GizmoController.hpp"

namespace {

[[nodiscard]] auto createSun() {
  return gfx::Light{
    .type = gfx::LightType::Directional,
    .visible = true,
    .position = {0.0f, 5.0f, 0.0f},
    .direction = {-0.551486731f, -0.827472687f, 0.105599940f},
    .color =
      {
        0.9294117647058824f,
        0.8352941176470588f,
        0.6196078431372549f,
      },
    .intensity = 5.0f,
    .shadowBias = 0.0005f,
  };
}

[[nodiscard]] auto getMesh(entt::id_type meshId) {
  auto &meshes = Services::Resources::Meshes::value();
  return meshes[meshId].handle();
}

[[nodiscard]] auto createGroundPlane() {
  gfx::MeshInstance mesh{getMesh(gfx::MeshManager::BasicShapes::Plane)};
  mesh.setTransform(Transform{}.setPosition({0.0f, -0.5f, 0.0f}));
  return mesh;
}

[[nodiscard]] auto getNumSubMeshes(const gfx::Mesh &mesh) {
  return static_cast<uint32_t>(mesh.getSubMeshes().size());
}

} // namespace

//
// MaterialPreviewWidget class:
//

MaterialPreviewWidget::MaterialPreviewWidget(os::InputSystem &inputSystem,
                                             gfx::WorldRenderer &renderer)
    : RenderTargetPreview{renderer.getRenderDevice()},
      m_inputSystem{inputSystem}, m_renderer{renderer} {
  m_renderSettings.shadow.cascadedShadowMaps.lambda = 0.95f;
  m_renderSettings.debugFlags |= gfx::DebugFlags::InfiniteGrid;
  m_renderSettings.features |= gfx::RenderFeatures::EyeAdaptation;

  m_camera.invertY(true)
    .setFov(60.0f)
    .setClippingPlanes({.zNear = 0.1f, .zFar = 100.0f})
    .fromTransform(
      Transform{}.setPosition({-2.0f, 1.0f, 2.0f}).lookAt(m_meshTransform));

  if (auto envMap = gfx::TextureLoader{}(
        "./assets/MaterialEditor/DefaultEnvironmentMap.hdr",
        m_renderer.getRenderDevice());
      envMap) {
    m_skyLight = m_renderer.createSkyLight(gfx::TextureResourceHandle{envMap});
  }

  m_light = createSun();
  m_groundPlane = createGroundPlane();
  setMesh(gfx::MeshManager::BasicShapes::Sphere);
}

gfx::WorldRenderer &MaterialPreviewWidget::getRenderer() const {
  return m_renderer;
}

void MaterialPreviewWidget::setMesh(entt::id_type meshId) {
  std::shared_ptr<gfx::Material> previousMaterial;
  if (m_meshInstance)
    previousMaterial = m_meshInstance.getMaterial(0).getPrototype();

  m_meshInstance = gfx::MeshInstance{getMesh(meshId)};
  m_meshInstance.setTransform(m_meshTransform);
  if (previousMaterial) m_meshInstance.setMaterial(0, previousMaterial);

  m_materialIndex = 0;
}
void MaterialPreviewWidget::updateMaterial(std::shared_ptr<gfx::Material> m) {
  if (m) {
    if (isSurface(*m)) {
      m_meshInstance.setMaterial(0, std::move(m));
      m_postProcessEffect[0] = gfx::MaterialInstance{};
    } else {
      m_postProcessEffect[0] = gfx::MaterialInstance{std::move(m)};
      m_meshInstance.reset();
    }
  } else {
    m_meshInstance.reset();
    m_postProcessEffect[0] = gfx::MaterialInstance{};
  }
}

void MaterialPreviewWidget::onRender(rhi::CommandBuffer &cb, float dt) {
  RenderTargetPreview::render(cb, [this, &cb, dt](auto &texture) {
    std::array<const gfx::Light *, 1> lights{&m_light};
    std::array<const gfx::MeshInstance *, 2> meshes{
      &m_meshInstance,
      &m_groundPlane,
    };

    std::array sceneViews{gfx::SceneView{
      .name = "MaterialPreview",
      .target = texture,
      .camera = m_camera,
      .renderSettings = m_renderSettings,
      .skyLight = m_skyLight ? &m_skyLight : nullptr,
      .postProcessEffects = m_postProcessEffect,
    }};
    constexpr auto kWorldExtent = 100.0f;
    gfx::WorldView worldView{
      .aabb =
        {
          .min = glm::vec3{-kWorldExtent},
          .max = glm::vec3{kWorldExtent},
        },
      .meshes = std::span{meshes.begin(), m_showGroundPlane ? 2u : 1u},
      .sceneViews = sceneViews,
    };
    if (m_light.visible) worldView.lights = lights;

    m_renderer.drawFrame(cb, worldView, dt);
  });
}

void MaterialPreviewWidget::showPreview(const char *name) {
  constexpr auto kWindowFlags =
    ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBackground |
    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar;
  if (ImGui::Begin(name, nullptr, kWindowFlags)) {
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("Camera")) {
        if (ImGui::BeginMenu("Sensitivity")) {
          ImGui::PushItemWidth(100);
          ImGui::SliderFloat("Rotate", &m_cameraControllerSettings.rotateSpeed,
                             0.01f, 1.0f);
          ImGui::BeginDisabled(true);
          ImGui::SliderFloat("Pan", &m_cameraControllerSettings.panSpeed, 0.01f,
                             1.0f);
          ImGui::EndDisabled();
          ImGui::SliderFloat("Zoom", &m_cameraControllerSettings.zoomSpeed,
                             0.01f, 1.0f);
          ImGui::PopItemWidth();

          ImGui::Separator();

          ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
          if (ImGui::MenuItem("Reset")) m_cameraControllerSettings = {};
          ImGui::PopItemFlag();

          ImGui::EndMenu();
        }
        ImGui::SeparatorText("Pivot");
        ImGui::DragFloat3(IM_UNIQUE_ID, m_pivotOffset, 0.1f, 0.0f, FLT_MAX,
                          "%.3f");

        ImGui::EndMenu();
      }

      /*
      if (ImGui::BeginMenu("Gizmo")) {
        inspect(m_gizmoSettings);
        ImGui::EndMenu();
      }
      */

      if (ImGui::BeginMenu("Mesh")) {
        const auto resource =
          std::dynamic_pointer_cast<Resource>(m_meshInstance.getPrototype());

        const auto menuItem = [this, currentMeshId = resource->getResourceId()](
                                const char *label, entt::id_type meshId) {
          if (auto selected = currentMeshId == meshId;
              ImGui::MenuItem(label, nullptr, &selected, !selected)) {
            setMesh(meshId);
          }
        };

        using BasicShapes = gfx::MeshManager::BasicShapes;

        ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
        menuItem("Plane", BasicShapes::Plane);
        menuItem("SubdividedPlane", BasicShapes::SubdividedPlane);
        menuItem("Cube", BasicShapes::Cube);
        menuItem("Sphere", BasicShapes::Sphere);
        ImGui::PopItemFlag();

        ImGui::EndMenu();
      }

      if (ImGui::BeginMenu("Light")) {
        if (ImGui::MenuItem("Summon to camera")) {
          m_light.position = m_camera.getPosition();
          m_light.direction = m_camera.getForward();
        }
        ImGui::EndMenu();
      }

      ImGui::EndMenuBar();
    }

    RenderTargetPreview::show(
      [this](auto extent) { m_camera.setAspectRatio(extent.getAspectRatio()); },
      [this] {
        CameraController::Result controllerResult;

        if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered() &&
            !ImGuizmo::IsHovered()) {
          // Don't use MouseDelta form ImGui IO (causes jittering).
          controllerResult = CameraController::update(
            m_camera, m_cameraControllerSettings, m_inputSystem.getMouseDelta(),
            CameraController::ArcBallConfig{
              .pivot = m_meshTransform.getPosition() + m_pivotOffset,
              .viewport = ImGui::GetWindowSize(),
            });
        }
        if (controllerResult.dirty) {
          const auto anchor = ImGui::GetCurrentWindowCenter();
          m_inputSystem.setMousePosition(anchor);
        }
        m_inputSystem.showCursor(!controllerResult.wantUse);

        if (!controllerResult.wantUse) {
          if (GizmoController::update(m_camera, m_gizmoSettings,
                                      m_meshTransform)) {
            m_meshInstance.setTransform(m_meshTransform);
          }
        }
      });
  }
  ImGui::End();
}
void MaterialPreviewWidget::showSceneSettings(const char *name) {
  if (ImGui::Begin(name)) {
    ImGui::Checkbox("Ground plane", &m_showGroundPlane);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Light")) {
      ImGui::Frame([this] { inspect(m_light, true); });
    }
    if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Frame([this] {
        ImGui::Dummy({0, 2});

        const auto meshResource =
          std::dynamic_pointer_cast<Resource>(m_meshInstance.getPrototype());

        ImGui::Text(ICON_FA_FILE " Resource: %s",
                    meshResource ? toString(*meshResource).c_str() : "(none)");
        if (ImGui::BeginDragDropTarget()) {
          if (auto incomingResource = extractResourceFromPayload(
                kImGuiPayloadTypeMesh, Services::Resources::Meshes::value());
              incomingResource) {
            if (auto r = incomingResource->handle(); meshResource != r) {
              setMesh(r->getResourceId());
            }
          }
          ImGui::EndDragDropTarget();
        }

        ImGui::Dummy({0, 2});

        // --

        const auto kMinIndex = 0u;
        const auto maxIndex =
          getNumSubMeshes(*m_meshInstance.getPrototype()) - 1;

        ImGui::SetNextItemWidth(100);
        ImGui::SliderScalar("Material index", ImGuiDataType_U32,
                            &m_materialIndex, &kMinIndex, &maxIndex, nullptr,
                            ImGuiSliderFlags_AlwaysClamp);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::TreeNodeEx("MaterialInstance",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
          inspect(m_meshInstance.getMaterial(m_materialIndex));
          ImGui::TreePop();
        }

        // ---

        ImGui::SeparatorText("Transform");
        ImGui::PushItemWidth(150);
        if (inspect(m_meshTransform)) {
          m_meshInstance.setTransform(m_meshTransform);
          m_camera.fromTransform(Transform{}
                                   .setPosition(m_camera.getPosition())
                                   .lookAt(m_meshTransform));
        }
        ImGui::PopItemWidth();
      });
    }
    if (auto &effect = *m_postProcessEffect.begin(); effect) {
      if (ImGui::CollapsingHeader("PostProcess")) {
        ImGui::Frame([&effect] { inspect(effect); });
      }
    }
  }
  ImGui::End();
}
void MaterialPreviewWidget::showRenderSettings(const char *name) {
  if (ImGui::Begin(name)) {
    ImGui::Frame([this] { ::showRenderSettings(m_renderSettings); });
  }
  ImGui::End();
}
