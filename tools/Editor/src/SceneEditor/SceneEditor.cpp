#include "SceneEditor/SceneEditor.hpp"
#include "Services.hpp"

#include "Inspectors/CameraInspector.hpp"
#include "Inspectors/SkyLightInspector.hpp"
#include "Inspectors/PostProcessEffectInspector.hpp"

#include "ImGuiTitleBarMacro.hpp"
#include "ImGuiModal.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "imgui_internal.h" // DockBuilder, MenuItemEx

#include "FileDialog.hpp"
#include "TexturePreview.hpp"

#include "RenderSettings.hpp"

#include "HierarchySystem.hpp"
#include "PhysicsSystem.hpp"
#include "RenderSystem.hpp"
#include "AnimationSystem.hpp"
#include "ScriptSystem.hpp"

#include "glm/gtc/type_ptr.hpp" // value_ptr

#include "spdlog/spdlog.h"

#include <fstream> // ofstream
#include <ranges>

namespace {

#define TOOL_ID "##SceneEditor"

struct GUI {
  GUI() = delete;

  struct Windows {
    Windows() = delete;

    static constexpr auto kInspector = ICON_FA_EYE " Inspector" TOOL_ID;
    static constexpr auto kEntities = ICON_FA_CUBE " Entities" TOOL_ID;
    static constexpr auto kConfig = " Config" TOOL_ID;
    static constexpr auto kScenes = " Scenes" TOOL_ID;
  };

  struct Modals {
    Modals() = delete;

    static constexpr auto kSaveSceneAsId =
      MAKE_TITLE_BAR(ICON_FA_FLOPPY_DISK, "Save As ...") TOOL_ID;
    static constexpr auto kOpenSceneId =
      MAKE_TITLE_BAR(ICON_FA_UPLOAD, "Open") TOOL_ID;
    static constexpr auto kCloseSceneId = MAKE_WARNING("Confirm") TOOL_ID;
  };
};

#undef TOOL_ID

[[nodiscard]] auto makeLabel(const entt::entity e) {
  return std::format("{} ({})", entt::to_entity(e), entt::to_version(e));
}
[[nodiscard]] auto makeLabel(const entt::handle h) {
  auto *c = h.try_get<const NameComponent>();
  return c && !c->name.empty() ? c->name : makeLabel(h.entity());
}

void embedEntity(entt::entity e) {
  ImGui::SetDragDropPayload(kImGuiPayloadTypeEntity, &e, sizeof(decltype(e)));
}
[[nodiscard]] auto extractEntity(const ImGuiPayload &payload) {
  return *static_cast<const entt::entity *>(payload.Data);
}

void print(const entt::entity e) {
  ImGui::BulletText("ID: %u", entt::to_entity(e));
  ImGui::BulletText("Version: %hu", entt::to_version(e));
}

void setupEditorViewport(SceneEditor::Viewport &viewport) {
  viewport.camera.invertY(true)
    .setFov(60.0f)
    .setClippingPlanes({.zNear = 0.1f, .zFar = 1000.0f})
    .setPosition({0.0f, 3.0f, -10.0f});
  viewport.renderSettings.debugFlags |= gfx::DebugFlags::InfiniteGrid;
}
void setupSimpleScene(Scene &scene) {
  auto &meshManager = Services::Resources::Meshes::value();
  using BasicShapes = gfx::MeshManager::BasicShapes;

  auto h = scene.createEntity("GroundPlane");
  h.emplace<Transform>()
    .setPosition(glm::vec3{0.0f, -1.0f, 0.0f})
    .scale(glm::vec3{5.0f});
  h.emplace<gfx::MeshInstance>(meshManager[BasicShapes::Plane].handle());

  h = scene.createEntity("Cube");
  h.emplace<Transform>().setPosition({0.0f, 1.0f, 0.0f});
  h.emplace<gfx::MeshInstance>(meshManager[BasicShapes::Cube].handle());

  h = scene.createEntity("Sun");
  h.emplace<Transform>().lookAt({0.5f, -0.5f, 0.0f, 0.0f});
  constexpr glm::vec3 kSunColor{
    0.9294117647058824f,
    0.8352941176470588f,
    0.6196078431372549f,
  };
  h.emplace<gfx::Light>(gfx::Light{
    .type = gfx::LightType::Directional,
    .visible = true,
    .color = kSunColor,
    .intensity = 2.3f,
    .shadowBias = 0.0f,
  });

  h = scene.createEntity("Camera");
  h.emplace<Transform>().setPosition({0.0f, 1.0f, -5.0f});
  h.emplace<CameraComponent>(rhi::Extent2D{1280, 720})
    .camera.setClippingPlanes({.zNear = 0.1f, .zFar = 1000.0f});

  getMainCamera(*h.registry()).e = h;
}

void inspect(AABB &aabb) {
  auto center = aabb.getCenter();
  auto dirty = ImGui::InputFloat3("Center", glm::value_ptr(center), "%.3f",
                                  ImGuiInputTextFlags_EnterReturnsTrue);
  auto halfExtents = aabb.getExtent() * 0.5f;
  dirty |= ImGui::InputFloat3("Half Extents", glm::value_ptr(halfExtents),
                              "%.3f", ImGuiInputTextFlags_EnterReturnsTrue);
  if (dirty) aabb = AABB::create(center, halfExtents);
}

void inspect(PhysicsWorld &physicsWorld) {
  if (auto v = physicsWorld.getGravity(); ImGui::DragFloat3("Gravity", v)) {
    physicsWorld.setGravity(v);
  }
  if (auto b = physicsWorld.isDebugDrawEnabled();
      ImGui::Checkbox("DebugDraw", &b)) {
    physicsWorld.enableDebugDraw(b);
  }

  ImGui::Spacing();
  ImGui::SeparatorText("Stats");

  constexpr auto kTableFlags =
    ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInner |
    ImGuiTableFlags_BordersOuter | ImGuiTableFlags_SizingStretchProp;
  if (ImGui::BeginTable(IM_UNIQUE_ID, 2, kTableFlags)) {
    ImGui::TableSetupColumn("MotionType");
    ImGui::TableSetupColumn("Count");
    ImGui::TableHeadersRow();

    const auto printRow = [](const char *label, uint32_t count) {
      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text(label);
      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%u", count);
    };

    const auto stats = physicsWorld.getJolt().GetBodyStats();
    printRow("Dynamic", stats.mNumBodiesDynamic);
    printRow("Static", stats.mNumBodiesStatic);
    printRow("Kinematic", stats.mNumBodiesKinematic);

    ImGui::EndTable();
  }
}

void inspect(SceneEditor::Viewport &viewport, gfx::WorldRenderer &renderer) {
  constexpr auto kTreeNodeFlags = ImGuiTreeNodeFlags_Bullet;
  if (ImGui::CollapsingHeader("Camera", kTreeNodeFlags)) {
    ImGui::Frame([&viewport] { ::inspect(viewport.camera, true); });
  }
  if (ImGui::CollapsingHeader("RenderSettings", kTreeNodeFlags)) {
    ImGui::Frame([&viewport] { showRenderSettings(viewport.renderSettings); });
  }
  if (ImGui::CollapsingHeader("SkyLight", kTreeNodeFlags)) {
    ImGui::Frame(
      [&viewport, &renderer] { ::inspect(viewport.skyLight, renderer); });
  }
  if (ImGui::CollapsingHeader("PostProcess", kTreeNodeFlags)) {
    ImGui::Frame(
      [&viewport] { inspectPostProcessEffects(viewport.postProcessEffects); });
  }
}

void cameraOverlay(entt::handle h, float height = 128) {
  if (h) {
    if (auto *cc = h.try_get<const CameraComponent>(); cc && cc->extent) {
      const auto width =
        glm::clamp(height * cc->camera.getAspectRatio(), height, 2.0f * height);
      overlay(cc->target.get(), {width, height});
    }
  }
}
void showSceneViewportInspector(os::InputSystem &inputSystem,
                                RenderTargetPreview &renderTargetPreview,
                                SceneEditor::Entry &entry, bool showOverlay) {
  renderTargetPreview.show(
    [&entry](const auto extent) {
      entry.viewport.camera.setAspectRatio(extent.getAspectRatio());
    },
    [&inputSystem, &entry, showOverlay] {
      CameraController::Result controller;

      auto &camera = entry.viewport.camera;
      if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered() &&
          !ImGuizmo::IsHovered()) {
        controller =
          CameraController::update(camera, entry.cameraControllerSettings,
                                   inputSystem.getMouseDelta(), std::nullopt);
      }
      if (controller.dirty) {
        const auto anchor = ImGui::GetCurrentWindowCenter();
        inputSystem.setMousePosition(anchor);
      }
      inputSystem.showCursor(!controller.wantUse);

      // ---

      auto h = entry.selectedEntity;
      if (!controller.wantUse && h) {
        if (GizmoController::update(camera, entry.gizmoSettings, h)) {
          PhysicsSystem::updateTransform(*h.registry(), h);
        }
      }
      if (showOverlay) cameraOverlay(h);
    });
}

void showScenePreview(Scene &scene, RenderTargetPreview &renderTargetPreview) {
  if (auto *cc = getMainCameraComponent(scene.getRegistry()); cc) {
    renderTargetPreview.show(
      [&cc](const auto extent) {
        cc->camera.setAspectRatio(extent.getAspectRatio());
      },
      std::nullopt);
  } else {
    ImGui::BeginDisabled(true);
    ImGui::Button("(Main camera is not set)", ImGui::GetContentRegionAvail());
    ImGui::EndDisabled();
  }
}

void showComponentsMenuItems(entt::handle h) {
  // WARNING:
  // Complex components can not be replaced! remove and then emplace.

  if (ImGui::MenuItemEx("Name", ICON_FA_ID_BADGE)) {
    h.emplace_or_replace<NameComponent>();
  }

  if (ImGui::MenuItemEx("Transform", ICON_FA_LOCATION_ARROW)) {
    h.emplace_or_replace<Transform>();
  }
  if (ImGui::MenuItemEx("Script", ICON_FA_CODE)) {
    h.remove<ScriptComponent>();
    h.emplace<ScriptComponent>();
  }

  if (ImGui::BeginMenu("Renderer")) {
    if (ImGui::MenuItemEx("CameraComponent", ICON_FA_VIDEO)) {
      h.remove<CameraComponent>();
      h.emplace<CameraComponent>();
    }
    if (ImGui::MenuItemEx("MeshInstance", ICON_FA_SHAPES)) {
      h.emplace_or_replace<gfx::MeshInstance>();
    }
    if (ImGui::MenuItemEx("DecalInstance", ICON_FA_NOTE_STICKY)) {
      h.emplace_or_replace<gfx::DecalInstance>();
    }

    if (ImGui::BeginMenu("Lights")) {
      if (ImGui::MenuItemEx("DirLight", ICON_FA_SUN)) {
        h.emplace_or_replace<gfx::Light>(gfx::Light{
          .type = gfx::LightType::Directional,
          .shadowBias = 0.0f,
        });
      }
      if (ImGui::MenuItemEx("SpotLight", ICON_FA_LIGHTBULB)) {
        h.emplace_or_replace<gfx::Light>(gfx::Light{
          .type = gfx::LightType::Spot,
          .shadowBias = 0.0f,
        });
      }
      if (ImGui::MenuItemEx("PointLight", ICON_FA_LIGHTBULB)) {
        h.emplace_or_replace<gfx::Light>(gfx::Light{
          .type = gfx::LightType::Point,
          .shadowBias = 0.0f,
        });
      }

      ImGui::EndMenu();
    }

    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Skeletal Animation")) {
    if (ImGui::MenuItemEx("Skeleton", ICON_FA_BONE)) {
      h.emplace_or_replace<SkeletonComponent>();
    }
    if (ImGui::MenuItemEx("Animation", ICON_FA_PERSON_WALKING)) {
      h.emplace_or_replace<AnimationComponent>();
    }
    if (ImGui::MenuItemEx("Controller", ICON_FA_CLAPPERBOARD)) {
      h.emplace_or_replace<PlaybackController>();
    }

    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Physics")) {
    if (ImGui::MenuItem("ColliderComponent")) {
      h.emplace_or_replace<ColliderComponent>();
    }

    const auto *collider = h.try_get<ColliderComponent>();
    const auto hasValidCollider = collider && collider->resource;

    if (ImGui::BeginMenu("RigidBody", hasValidCollider)) {
      if (ImGui::MenuItem("Static")) {
        h.remove<RigidBody>();
        h.emplace<RigidBody>(RigidBody{
          RigidBodySettings{
            .motionType = MotionType::Static,
          },
        });
      }
      if (ImGui::MenuItem("Kinematic")) {
        h.remove<RigidBody>();
        h.emplace<RigidBody>(RigidBody{
          RigidBodySettings{
            .motionType = MotionType::Kinematic,
          },
        });
      }
      if (ImGui::MenuItem("Dynamic")) {
        h.remove<RigidBody>();
        h.emplace<RigidBody>(RigidBody{
          RigidBodySettings{
            .motionType = MotionType::Dynamic,
          },
        });
      }
      ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Character", nullptr, nullptr, hasValidCollider)) {
      h.remove<Character>();
      h.emplace<Character>();
    }

    ImGui::EndMenu();
  }
}

[[nodiscard]] gfx::SceneView
createEditorSceneView(SceneEditor::Viewport &viewport, rhi::Texture &target) {
  auto &[camera, renderSettings, skyLight, postProcessEffects, debugDraw] =
    viewport;

  return {
    .name = "EditorViewport",
    .target = target,
    .camera = camera,
    .renderSettings = renderSettings,
    .skyLight = skyLight ? &skyLight : nullptr,
    .postProcessEffects = postProcessEffects,
    .debugDraw = &debugDraw,
  };
}

} // namespace

//
// SceneEditor class:
//

SceneEditor::SceneEditor(os::InputSystem &is, rhi::RenderDevice &rd)
    : m_inputSystem{is}, m_renderDevice{rd}, m_renderTargetPreview{rd} {
  _connectInspectors2<NameComponent, ParentComponent, ChildrenComponent,
                      Transform>();
  _connectInspectors(PhysicsSystem::kIntroducedComponents);
  _connectInspectors(RenderSystem::kIntroducedComponents);
  _connectInspectors(AnimationSystem::kIntroducedComponents);
  _connectInspectors(ScriptSystem::kIntroducedComponents);

  m_dispatcher.sink<DetachEntityRequest>().connect<&SceneEditor::_detachEntity>(
    this);
  m_dispatcher.sink<SelectEntityRequest>().connect<&SceneEditor::_selectEntity>(
    this);

  ImGuizmo::StyleColorsBlender();
}

SceneEditor::Entry *SceneEditor::getActiveSceneEntry() {
  return m_activeSceneId ? m_scenes[*m_activeSceneId].get() : nullptr;
}

Scene *SceneEditor::getCurrentScene() {
  Scene *scene{nullptr};
  if (auto *entry = getActiveSceneEntry(); entry) {
    scene = &entry->scene;
  } else if (m_playTest) {
    scene = &m_playTest.value();
  }
  return scene;
}
ScriptContext *SceneEditor::getScriptContext() {
  if (auto *scene = getCurrentScene(); scene) {
    return &scene->getRegistry().ctx().get<ScriptContext>();
  }
  return nullptr;
}

void SceneEditor::closeAllScenes() {
  m_scenes.clear();
  m_activeSceneId = std::nullopt;
}

void SceneEditor::expose(sol::state &lua) {
  lua["inEditor"] = sol::readonly_property([] { return true; });

  lua["getSelectedEntity"] = [this] {
    const auto *entry = getActiveSceneEntry();
    return entry ? entry->selectedEntity : entt::handle{};
  };
}

void SceneEditor::show(const char *name, bool *open) {
  ImGui::Begin(name, open);
  const auto dockspaceId = ImGui::GetID("DockSpace");
  _setupDockSpace(dockspaceId);
  ImGui::DockSpace(dockspaceId);
  ImGui::End();

  if (auto *entry = getActiveSceneEntry(); entry) {
    _showConfigWidget(*entry);
    _showEntitiesWidget(*entry);
  }

  ImGuiWindowClass wc;
  wc.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_AutoHideTabBar;
  ImGui::SetNextWindowClass(&wc);
  if (ImGui::Begin(GUI::Windows::kScenes, nullptr,
                   ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoMove)) {
    _menuBar();

    constexpr auto kSceneExtension = ".scene";
    constexpr auto sceneFilter = makeExtensionFilter(kSceneExtension);

    const auto &rootDir = os::FileSystem::getRoot();
    static auto currentDir = rootDir;
    if (const auto p =
          showFileDialog(GUI::Modals::kSaveSceneAsId,
                         {
                           .dir = currentDir,
                           .barrier = rootDir,
                           .entryFilter = sceneFilter,
                           .forceExtension = kSceneExtension,
                           .flags = FileDialogFlags_AskOverwrite |
                                    FileDialogFlags_CreateDirectoryButton,
                         });
        p) {
      auto &entry = m_scenes[*m_activeSceneId];
      if (entry->scene.save(*p, Scene::ArchiveType::JSON)) {
        entry->path = *p;
      }
    }
    if (const auto p = showFileDialog(GUI::Modals::kOpenSceneId,
                                      {
                                        .dir = currentDir,
                                        .barrier = rootDir,
                                        .entryFilter = sceneFilter,
                                      });
        p) {
      _openScene(*p);
    }
    if (const auto button =
          showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
            GUI::Modals::kCloseSceneId,
            "Do you really want to close the current scene?");
        button && *button == ModalButton::Yes) {
      m_scenes.erase(m_scenes.begin() + *m_activeSceneId);
      m_activeSceneId = std::nullopt;
    }

    const auto cursorPos = ImGui::GetCursorPos();
    // FEATURE:
    // Drag'n'Drop a file from the FileBrowser to open a scene in a new tab.
    ImGui::Dummy(ImGui::GetContentRegionAvail());
    onDropTarget(kImGuiPayloadTypeFile, [this](const ImGuiPayload *payload) {
      _openScene(ImGui::ExtractPath(*payload));
    });
    ImGui::SetCursorPos(cursorPos);
    _scenesWidget();
  }
  ImGui::End();

  // ImGuizmo::PrintContext();
}

void SceneEditor::onInput(const os::InputEvent &evt) {
  if (m_passthroughInput) {
    assert(m_playTest);
    ScriptSystem::onInput(m_playTest->getRegistry(), evt);
  }
}
void SceneEditor::onUpdate(float dt) {
  m_dispatcher.update();
  if (m_playTest) {
    auto &r = m_playTest->getRegistry();
    AnimationSystem::update(r, dt);
    ScriptSystem::onUpdate(r, dt);
  }
}
void SceneEditor::onPhysicsUpdate(float dt) {
  if (m_playTest) {
    auto &r = m_playTest->getRegistry();
    PhysicsSystem::simulate(r, dt);
    ScriptSystem::onPhysicsStep(r, dt);
  }
}
void SceneEditor::onRender(rhi::CommandBuffer &cb, float dt) {
  m_renderTargetPreview.render(
    cb, [this, &cb, dt](auto &texture) { _drawWorld(cb, texture, dt); });
}

//
// (private):
//

SceneEditor::Entry &SceneEditor::_addScene() {
  auto &entry = m_scenes.emplace_back(std::make_unique<Entry>());
  publish(NewSceneEvent{.scene = &entry->scene});
  setupSimpleScene(entry->scene);
  setupEditorViewport(entry->viewport);
  return *entry;
}
void SceneEditor::_openScene(const std::filesystem::path &p) {
  if (_hasScene(p)) return;

  auto entry = std::make_unique<Entry>();
  publish(NewSceneEvent{.scene = &entry->scene});
  setupEditorViewport(entry->viewport);

  const auto relativePath = os::FileSystem::relativeToRoot(p);
  if (entry->scene.load(p, Scene::ArchiveType::JSON)) {
    entry->path = p;
    m_scenes.emplace_back(std::move(entry));

    SPDLOG_INFO("Scene loaded: {}", relativePath->string());
  } else {
    SPDLOG_ERROR("Could not load a scene: {}", relativePath->string());
  }
}
bool SceneEditor::_hasScene(const std::filesystem::path &p) const {
  assert(!p.empty());
  const auto it = std::ranges::find_if(
    m_scenes, [&p](const auto &entry) { return entry->path == p; });
  return it != m_scenes.cend();
}

void SceneEditor::_selectEntity(const SelectEntityRequest &req) {
  if (auto *entry = getActiveSceneEntry(); entry) entry->selectedEntity = req.h;
}
void SceneEditor::_detachEntity(const DetachEntityRequest &req) {
  HierarchySystem::detach(*req.h.registry(), req.h);
}

void SceneEditor::_setupDockSpace(const ImGuiID dockspaceId) {
  if (static auto firstTime = true; firstTime) [[unlikely]] {
    firstTime = false;

    if (ImGui::DockBuilderGetNode(dockspaceId) == nullptr) {
      ImGui::DockBuilderRemoveNode(dockspaceId);
      ImGui::DockBuilderAddNode(dockspaceId);

      auto centerNodeId = dockspaceId;

      auto rightNodeId = ImGui::DockBuilderSplitNode(
        centerNodeId, ImGuiDir_Right, 0.21f, nullptr, &centerNodeId);
      const auto rightBottomNodeId = ImGui::DockBuilderSplitNode(
        rightNodeId, ImGuiDir_Down, 0.45f, nullptr, &rightNodeId);

      ImGui::DockBuilderDockWindow(GUI::Windows::kEntities, rightNodeId);
      ImGui::DockBuilderDockWindows(
        {
          GUI::Windows::kInspector,
          GUI::Windows::kConfig,
        },
        rightBottomNodeId);
      ImGui::DockBuilderDockWindow(GUI::Windows::kScenes, centerNodeId);

      ImGui::DockBuilderFinish(dockspaceId);
    }
  }
}

void SceneEditor::_menuBar() {
  std::optional<const char *> action;
  if (ImGui::BeginMenuBar()) {
    auto *activeEntry = getActiveSceneEntry();
    const auto hasScene = activeEntry != nullptr;

    if (ImGui::BeginMenu("Scene")) {
      if (ImGui::MenuItemEx("New ...", ICON_FA_FILE)) {
        _addScene();
      }
      if (ImGui::MenuItemEx("Save", ICON_FA_DOWNLOAD, nullptr, false,
                            hasScene)) {
        if (const auto &p = activeEntry->path; p.empty()) {
          action = GUI::Modals::kSaveSceneAsId;
        } else {
          activeEntry->scene.save(p, Scene::ArchiveType::JSON);
        }
      }
      if (ImGui::MenuItemEx("Open", ICON_FA_UPLOAD)) {
        action = GUI::Modals::kOpenSceneId;
      }
      ImGui::Separator();
      if (ImGui::MenuItemEx("Close", ICON_FA_XMARK, nullptr, false, hasScene)) {
        action = GUI::Modals::kCloseSceneId;
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Debug")) {
      if (ImGui::BeginMenuEx("FrameGraph (dot)", ICON_FA_DIAGRAM_PROJECT,
                             hasScene || m_playTest)) {
        if (ImGui::MenuItemEx("File", ICON_FA_FILE))
          m_frameGraphDebugOutput = FrameGraphDebugOutput::File;
        if (ImGui::MenuItemEx("Clipboard", ICON_FA_CLIPBOARD))
          m_frameGraphDebugOutput = FrameGraphDebugOutput::Clipboard;

        ImGui::EndMenu();
      }
      ImGui::EndMenu();
    }

    if (hasScene) {
      ImGui::Separator();
      ImGui::BeginDisabled(!activeEntry->selectedEntity);
      showBar(activeEntry->gizmoSettings);
      ImGui::EndDisabled();
    }

    ImGui::Separator();

    if (ImGui::MenuItem(ICON_FA_PLAY " Play", nullptr, nullptr,
                        hasScene && !m_playTest)) {
      publish(NewSceneEvent{&m_playTest.emplace()});

      auto &srcScene = getActiveSceneEntry()->scene;
      m_playTest->copyFrom(srcScene);

      getMainCamera(m_playTest->getRegistry()).e =
        getMainCamera(srcScene.getRegistry()).e;
    }
    if (ImGui::MenuItem(ICON_FA_STOP " Stop", nullptr, nullptr,
                        m_playTest.has_value())) {
      m_playTest = std::nullopt;
      m_passthroughInput = false;
    }

    ImGui::EndMenuBar();
  }

  if (action) ImGui::OpenPopup(*action);
}
void SceneEditor::_scenesWidget() {
  static std::optional<std::size_t> junk{};

  ImGui::BeginTabBar(IM_UNIQUE_ID, ImGuiTabBarFlags_AutoSelectNewTabs);

  for (auto [i, entry] : std::views::enumerate(m_scenes)) {
    auto open = true;
    const auto label = std::format(
      "{}##{}",
      entry->path.empty() ? "(NewScene)" : entry->path.filename().string(), i);
    const auto visible = ImGui::BeginTabItem(
      label.c_str(), &open,
      entry->path.empty() ? ImGuiTabItemFlags_UnsavedDocument
                          : ImGuiTabItemFlags_None);
    if (!open) junk = i;

    if (visible) {
      const auto last = std::exchange(m_activeSceneId, i);
      const auto changed = !last || last.value() != i;
      auto activeEntry = getActiveSceneEntry();
      if (changed) {
        activeEntry->viewport.camera.setAspectRatio(
          m_renderTargetPreview.getExtent().getAspectRatio());
      }
      showSceneViewportInspector(m_inputSystem, m_renderTargetPreview,
                                 *activeEntry, !junk);

      ImGui::EndTabItem();
    }

    ++i;
  }

  if (junk) ImGui::OpenPopup(GUI::Modals::kCloseSceneId);

  if (const auto button =
        showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
          GUI::Modals::kCloseSceneId,
          "Do you really want to close active scene?");
      button) {
    if (*button == ModalButton::Yes) {
      assert(junk.has_value());
      m_scenes.erase(m_scenes.begin() + *junk);
      if (m_activeSceneId && *m_activeSceneId == *junk) {
        // Activate previous tab (if possible).
        m_activeSceneId =
          m_scenes.empty() ? std::nullopt : std::optional{m_scenes.size() - 1};
      }
    }
    junk = std::nullopt;
  }

  if (m_playTest) {
    if (const auto visible =
          ImGui::BeginTabItem(ICON_FA_GAMEPAD " Game##PREVIEW");
        visible) {
      showScenePreview(*m_playTest, m_renderTargetPreview);
      m_passthroughInput =
        ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
      ImGui::EndTabItem();

      m_activeSceneId = std::nullopt;
    }
  }

  ImGui::EndTabBar();
}

void SceneEditor::_showConfigWidget(Entry &e) {
  if (ImGui::Begin(GUI::Windows::kConfig)) {
    auto &r = e.scene.getRegistry();

    if (ImGui::CollapsingHeader("Physics")) {
      ImGui::Frame([&r] { inspect(getPhysicsWorld(r)); });
    }
    if (ImGui::CollapsingHeader("Bounds")) {
      ImGui::Frame([&r] { inspect(r.ctx().get<AABB>()); });
    }
    if (ImGui::CollapsingHeader("Editor Viewport")) {
      constexpr auto kWidth = 8.0f;
      ImGui::Indent(kWidth);
      inspect(e.viewport, getRenderer(r));
      ImGui::Unindent(kWidth);
    }
  }
  ImGui::End();
}

void SceneEditor::_showEntitiesWidget(Entry &entry) {
  auto &scene = entry.scene;
  auto &selectedEntity = entry.selectedEntity;

  if (ImGui::Begin(GUI::Windows::kEntities)) {
    if (ImGui::Button(ICON_FA_PLUS " Create")) {
      scene.createEntity();
    }
    if (selectedEntity) {
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_CLONE " Clone")) {
        selectedEntity = scene.clone(selectedEntity);
      }

      constexpr auto kDestroyEntityPopupId = MAKE_WARNING("Confirm");
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_TRASH " Destroy"))
        ImGui::OpenPopup(kDestroyEntityPopupId);

      if (const auto button =
            showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
              kDestroyEntityPopupId, "Destroy selected entity?");
          button && *button == ModalButton::Yes) {
        selectedEntity.destroy();
      }
    }

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_BROOM " Cleanup")) {
      auto &r = scene.getRegistry();
      for (auto [e] : r.storage<entt::entity>().each()) {
        if (r.orphan(e)) r.destroy(e);
      }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    for (auto [e] : scene.each()) {
      _viewEntity(scene.get(e), selectedEntity, 0);
    }

    if (ImGui::Begin(GUI::Windows::kInspector) && selectedEntity) {
      _inspectorWidget(selectedEntity);
    }
    ImGui::End();
  }
  ImGui::End();
}

void SceneEditor::_viewEntity(entt::handle h, entt::handle &selected,
                              int32_t level) {
  const auto parent = getParent(h);
  if (level == 0 && parent) return;

  ImGui::PushID(entt::to_integral(h.entity()));

  auto *childrenComponent = h.try_get<const ChildrenComponent>();

  int32_t flags{
    ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick,
  };
  if (!childrenComponent || childrenComponent->children.empty()) {
    flags |= ImGuiTreeNodeFlags_Leaf;
  }
  if (h == selected) flags |= ImGuiTreeNodeFlags_Selected;

  constexpr auto kContextMenuId = IM_UNIQUE_ID;

  const auto label = makeLabel(h);
  const auto isOpen = ImGui::TreeNodeEx(label.c_str(), flags);
  if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
    selected = flags & ImGuiTreeNodeFlags_Selected ? entt::handle{} : h;
  } else if (parent && ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
    ImGui::OpenPopup(kContextMenuId);
  }

  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
    embedEntity(h);
    ImGui::Text("Entity: %s", label.c_str());
    ImGui::EndDragDropSource();
  }
  if (ImGui::BeginDragDropTarget()) {
    if (const auto *payload =
          ImGui::AcceptDragDropPayload(kImGuiPayloadTypeEntity)) {
      const auto childEntity = extractEntity(*payload);
      HierarchySystem::attachTo(*h.registry(), childEntity, h);
    }
    ImGui::EndDragDropTarget();
  }

  if (ImGui::BeginPopup(kContextMenuId, ImGuiWindowFlags_NoMove)) {
    if (ImGui::MenuItem("Detach")) {
      m_dispatcher.enqueue<DetachEntityRequest>(h);
    }
    ImGui::EndPopup();
  }

  if (isOpen) {
    if (childrenComponent) {
      for (const auto e : childrenComponent->children) {
        _viewEntity(entt::handle{*h.registry(), e}, selected, level + 1);
      }
    }
    ImGui::TreePop();
  }

  ImGui::PopID();
}
void SceneEditor::_inspectorWidget(entt::handle &h) {
  assert(h);

  ImGui::PushID(entt::to_integral(h.entity()));

  ImGui::Frame([h] {
    ImGui::SeparatorText("Entity");
    print(h);
    ImGui::Dummy({0, 2});

    ImGui::PushStyleColor(ImGuiCol_Button,
                          ImVec4{ImColor::HSV(0.28f, 0.6f, 0.6f)});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          ImVec4{ImColor::HSV(0.28f, 0.7f, 0.7f)});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          ImVec4{ImColor::HSV(0.28f, 0.8f, 0.8f)});

    constexpr auto kAddComponentPopupId = IM_UNIQUE_ID;
    if (ImGui::Button(ICON_FA_PUZZLE_PIECE " Add")) {
      ImGui::OpenPopup(kAddComponentPopupId);
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine();
    ImGui::SeparatorText("Components");

    if (ImGui::BeginPopup(kAddComponentPopupId, ImGuiWindowFlags_NoMove)) {
      ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
      showComponentsMenuItems(h);
      ImGui::PopItemFlag();
      ImGui::EndPopup();
    }
  });

  constexpr auto kRemoveComponentModalId = MAKE_WARNING("Remove Component");

  for (auto [componentId, pool] : h.storage()) {
    if (componentId == entt::type_hash<entt::entity>::value()) {
      continue;
    }

    const auto componentName = std::string{pool.type().name()};
    auto metaType = entt::resolve(componentId);

    auto visible = true;
    ImGui::PushID(componentId);
    auto treeOpened = ImGui::CollapsingHeader(
      componentName.c_str(), &visible,
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick);

    if (!visible) ImGui::OpenPopup(kRemoveComponentModalId);

    if (const auto button =
          showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
            kRemoveComponentModalId,
            std::format(
              "Remove the following component? This cannot be undone.\n- {}",
              componentName)
              .c_str());
        button && *button == ModalButton::Yes) {
      const auto removed =
        invokeMetaFunc(metaType, MetaComponent::Functions::Remove, h);
      treeOpened = removed.cast<std::size_t>() == 0;
    }

    if (treeOpened) {
      ImGui::Frame([this, &metaType, h] {
        if (metaType) {
          if (const auto inspected = _onGui(metaType, h); !inspected) {
            ImGui::Text("inspector not found!");
          }
        } else {
          ImGui::Text("meta_type not found!");
        }
      });
    }
    ImGui::PopID();
  }

  ImGui::PopID();
}

void SceneEditor::_drawWorld(rhi::CommandBuffer &cb, rhi::Texture &texture,
                             float dt) {
  static gfx::DebugOutput debugOutput;
  if (auto *entry = getActiveSceneEntry(); entry) {
    _drawWorld(*entry, cb, texture, dt,
               m_frameGraphDebugOutput ? &debugOutput : nullptr);
  } else if (m_playTest) {
    _drawWorld(*m_playTest, cb, texture, dt,
               m_frameGraphDebugOutput ? &debugOutput : nullptr);
  }
  if (m_frameGraphDebugOutput) {
    switch (*m_frameGraphDebugOutput) {
      using enum FrameGraphDebugOutput;

    case File:
      if (std::ofstream f{"fg.dot"}; f.is_open()) {
        f << debugOutput.dot;
      }
      break;

    case Clipboard:
      ImGui::SetClipboardText(debugOutput.dot.c_str());
      break;
    }
    m_frameGraphDebugOutput = std::nullopt;
  }
}
void SceneEditor::_drawWorld(Entry &entry, rhi::CommandBuffer &cb,
                             rhi::Texture &texture, float dt,
                             gfx::DebugOutput *debugOutput) {
  auto &r = entry.scene.getRegistry();
  const auto mainSceneView = [&entry, &texture, &r] {
    auto *c = getMainCameraComponent(r);
    return c ? createSceneView("Main", *c, &entry.viewport.camera, &texture)
             // Fallback to editor viewport:
             : createEditorSceneView(entry.viewport, texture);
  }();

  if (auto *dd = mainSceneView.debugDraw; dd) {
    PhysicsSystem::debugDraw(r, *dd);
  }
  RenderSystem::update(r, cb, dt, &mainSceneView, debugOutput);
}
void SceneEditor::_drawWorld(Scene &scene, rhi::CommandBuffer &cb,
                             rhi::Texture &texture, float dt,
                             gfx::DebugOutput *debugOutput) {
  auto &r = scene.getRegistry();
  if (auto *mainCamera = getMainCameraComponent(r); mainCamera) {
    PhysicsSystem::debugDraw(r, mainCamera->debugDraw);
    // AnimationSystem::debugDraw(r, mainCamera->debugDraw);
    RenderSystem::update(r, cb, dt, nullptr, debugOutput);
    if (auto *src = mainCamera->target.get(); src) {
      cb.blit(*src, texture, VK_FILTER_LINEAR);
    }
  }
}
