#pragma once

#include "WidgetWindow.hpp"
#include "Inspector.hpp"
#include "os/InputSystem.hpp"

#include "Scene.hpp"

#include "RenderTargetPreview.hpp"
#include "CameraController.hpp"
#include "GizmoController.hpp"

#include "ScriptContext.hpp"

#include "RmlUiPlatformInterface.hpp"
#include "RmlUiRenderInterface.hpp"

#include "entt/signal/dispatcher.hpp"
#include "sol/forward.hpp"

class SceneEditor final : public WidgetWindow,
                          public Inspector<SceneEditor>,
                          private entt::emitter<SceneEditor> {
  friend class Inspector<SceneEditor>;
  friend class entt::emitter<SceneEditor>;

public:
  using entt::emitter<SceneEditor>::on;

  struct Viewport {
    Viewport();

    gfx::PerspectiveCamera camera;
    gfx::RenderSettings renderSettings;
    gfx::SkyLight skyLight;
    std::vector<gfx::MaterialInstance> postProcessEffects;
    DebugDraw debugDraw;
  };
  struct Entry {
    Scene scene;
    std::filesystem::path path;

    entt::handle selectedEntity;
    GizmoController::Settings gizmoSettings;
    CameraController::Settings cameraControllerSettings;

    Viewport viewport;
  };

  struct EditScriptRequest {
    std::shared_ptr<ScriptResource> resource;
  };

  SceneEditor(os::InputSystem &, gfx::WorldRenderer &, sol::state &);
  SceneEditor(const SceneEditor &) = delete;
  ~SceneEditor() override;

  SceneEditor &operator=(const SceneEditor &) = delete;

  bool hasActiveScene() const { return m_activeSceneId.has_value(); }

  Entry *getActiveSceneEntry();
  Scene *getCurrentScene();

  void closeAllScenes();

  void show(const char *name, bool *open) override;

  void onInput(const os::InputEvent &) override;
  void onUpdate(float dt) override;
  void onPhysicsUpdate(float dt) override;
  void onRender(rhi::CommandBuffer &, float dt) override;

private:
  void _expose(sol::state &);

  [[nodiscard]] Entry _createEntry();
  Entry &_addScene();
  void _openScene(const std::filesystem::path &);
  [[nodiscard]] bool _hasScene(const std::filesystem::path &) const;

  struct DetachEntityRequest {
    entt::handle h;
  };
  struct SelectEntityRequest {
    entt::handle h;
  };

  void _selectEntity(const SelectEntityRequest &);
  void _detachEntity(const DetachEntityRequest &);

  // ---

  void _setupDockSpace(const ImGuiID dockspaceId);

  void _menuBar();
  void _scenesWidget();

  void _showConfigWidget(Entry &);

  void _showEntitiesWidget(Entry &);
  void _viewEntity(entt::handle inspected, entt::handle &selected,
                   int32_t level);
  void _inspectorWidget(entt::handle &);

  // ---

  void _drawWorld(rhi::CommandBuffer &, rhi::Texture &, float dt);
  void _drawWorld(Entry &, rhi::CommandBuffer &, rhi::Texture &, float dt,
                  gfx::DebugOutput *);
  void _drawWorld(Scene &, rhi::CommandBuffer &, rhi::Texture &, float dt,
                  gfx::DebugOutput *);

  // -- Component inspectors:

  void _onInspect(entt::handle, NameComponent &) const;

  void _onInspect(entt::handle, const ParentComponent &);
  void _onInspect(entt::handle, const ChildrenComponent &);
  void _onInspect(entt::handle, Transform &);

  void _onInspect(entt::handle, const ColliderComponent &) const;
  void _onInspect(entt::handle, const RigidBody &) const;
  void _onInspect(entt::handle, const Character &) const;
  void _onInspect(entt::handle, CharacterVirtual &) const;

  void _onInspect(entt::handle, SkeletonComponent &) const;
  void _onInspect(entt::handle, AnimationComponent &) const;
  void _onInspect(entt::handle, PlaybackController &) const;

  void _onInspect(entt::handle, CameraComponent &) const;
  void _onInspect(entt::handle, gfx::Light &) const;

  void _onInspect(entt::handle, gfx::MeshInstance &) const;
  void _onInspect(entt::handle, gfx::DecalInstance &) const;

  void _onInspect(entt::handle, const ScriptComponent &);

private:
  os::InputSystem &m_inputSystem;
  gfx::WorldRenderer &m_worldRenderer;
  sol::state &m_lua;

  entt::dispatcher m_dispatcher;

  enum class FrameGraphDebugOutput { File, Clipboard };
  std::optional<FrameGraphDebugOutput> m_frameGraphDebugOutput{};

  std::vector<std::unique_ptr<Entry>> m_scenes;
  std::optional<std::size_t> m_activeSceneId;

  bool m_passthroughInput{false};
  std::optional<Scene> m_playTest;

  RenderTargetPreview m_renderTargetPreview;

  std::unique_ptr<RmlUiFileInterface> m_uiFileInterface;
  std::unique_ptr<RmlUiSystemInterface> m_uiSystemInterface;
  std::unique_ptr<RmlUiRenderInterface> m_gameUiRenderInterface;
};
