#pragma once

#include "os/InputSystem.hpp"
#include "renderer/WorldRenderer.hpp"
#include "RenderTargetPreview.hpp"
#include "CameraController.hpp"
#include "SceneEditor/GizmoController.hpp"

class MaterialPreviewWidget final : public RenderTargetPreview {
public:
  MaterialPreviewWidget(os::InputSystem &, gfx::WorldRenderer &);

  gfx::WorldRenderer &getRenderer() const;

  void setMesh(entt::id_type meshId);
  void updateMaterial(std::shared_ptr<gfx::Material>);

  void onRender(rhi::CommandBuffer &, float dt);

  void showPreview(const char *name);
  void showSceneSettings(const char *name);
  void showRenderSettings(const char *name);

private:
  os::InputSystem &m_inputSystem;
  gfx::WorldRenderer &m_renderer;

  gfx::PerspectiveCamera m_camera;
  gfx::RenderSettings m_renderSettings;
  gfx::SkyLight m_skyLight;

  glm::vec3 m_pivotOffset{0.0f};
  CameraController::Settings m_cameraControllerSettings;
  GizmoController::Settings m_gizmoSettings{.operation =
                                              ImGuizmoOperation_None};

  // -- Simple scene:

  gfx::Light m_light;
  gfx::MeshInstance m_groundPlane;
  bool m_showGroundPlane{false};

  Transform m_meshTransform;
  gfx::MeshInstance m_meshInstance;
  uint32_t m_materialIndex{0};

  std::array<gfx::MaterialInstance, 1> m_postProcessEffect;
};
