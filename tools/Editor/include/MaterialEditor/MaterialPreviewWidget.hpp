#pragma once

#include "RenderTargetPreview.hpp"
#include "Transform.hpp"
#include "PerspectiveCamera.hpp"
#include "renderer/RenderSettings.hpp"
#include "renderer/Light.hpp"
#include "renderer/SkyLight.hpp"
#include "renderer/MeshInstance.hpp"
#include "renderer/MaterialInstance.hpp"
#include "CameraController.hpp"
#include "SceneEditor/GizmoController.hpp"

namespace os {
class InputSystem;
};

namespace gfx {
class WorldRenderer;
};

class MaterialPreviewWidget final : public RenderTargetPreview {
public:
  MaterialPreviewWidget(os::InputSystem &, gfx::WorldRenderer &);

  gfx::WorldRenderer &getRenderer() const;

  void setMesh(const entt::id_type meshId);
  void updateMaterial(std::shared_ptr<gfx::Material>);

  void onRender(rhi::CommandBuffer &, const float dt);

  void showPreview(const char *name, bool *open = nullptr);
  void showSceneSettings(const char *name, bool *open = nullptr);
  void showRenderSettings(const char *name, bool *open = nullptr);

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
