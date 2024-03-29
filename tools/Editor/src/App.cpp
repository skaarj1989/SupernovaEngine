#include "App.hpp"
#include "Services.hpp"

#include "MapOptional.hpp"

#include "physics/JoltPhysics.hpp"

#include "ScriptContext.hpp"
#include "LuaModules.hpp"

#include "ImGuiTitleBarMacro.hpp"
#include "ImGuiStyleJSON.hpp"
#include "FileDialog.hpp"
#include "FileBrowser.hpp"
#include "ResourceDockSpace.hpp"
#include "LoggerWidget.hpp"

#include "SceneEditor/SceneEditor.hpp"
#include "ScriptEditor.hpp"
#include "MaterialEditor/MaterialEditor.hpp"

#include "ShapeCreatorWidget.hpp"

#include "ImGuiModal.hpp"
#include "GPUWidget.hpp"
#include "WorldRendererWidget.hpp"
#include "ProjectSettingsWidget.hpp"
#include "imgui_internal.h" // DockBuilder

#pragma warning(push, 0)
#include "tracy/Tracy.hpp"
#include "tracy/TracyLua.hpp"
#pragma warning(pop)

namespace {

struct GUI {
  GUI() = delete;

  struct Modals {
    Modals() = delete;

    static constexpr auto kCloseEditorId = MAKE_WARNING("Close Editor");

    static constexpr auto kNewProjectId =
      MAKE_TITLE_BAR(ICON_FA_FILE, "New Project");
    static constexpr auto kOpenProjectId = "Open Project";
    static constexpr auto kCloseProjectId = MAKE_WARNING("Close Project");
    static constexpr auto kProjectSettingsId = "Project Settings";

    static constexpr auto kSaveThemeId =
      MAKE_TITLE_BAR(ICON_FA_PALETTE, "Save Theme");
    static constexpr auto kLoadThemeId =
      MAKE_TITLE_BAR(ICON_FA_PALETTE, "Load Theme");
  };
};

[[nodiscard]] auto createLuaState() {
  sol::state lua;

  using enum sol::lib;
  lua.open_libraries(base, package, string, os, math, table);
  tracy::LuaRegister(lua);
  registerModules(lua);

  const auto scriptLibraryPath = std::filesystem::current_path() / "lua/";
  lua["package"]["path"] =
    std::format("{}?.lua;", scriptLibraryPath.lexically_normal().string());

  return lua;
}

} // namespace

//
// App class:
//

App::App(std::span<char *> args)
    : ImGuiApp{
        args,
        {
          .caption = "Editor",
          //.vendor = rhi::Vendor::AMD,
          .verticalSync = true,
        },
      } {
#if 1
  auto &io = ImGui::GetIO();
  io.ConfigWindowsMoveFromTitleBarOnly = true;
  // io.IniFilename = nullptr;
#endif
  load("./assets/DarkTheme.json", ImGui::GetStyle());

  MetaComponent::registerMetaComponents(Scene::kComponentTypes);
  LuaComponent::extendMetaTypes(Scene::kComponentTypes);

  JoltPhysics::setup();

  m_audioDevice = std::make_unique<audio::Device>(audio::Device::Config{
    .maxNumSources = 10'000,
  });

  auto &rd = getRenderDevice();
  Services::init(rd, *m_audioDevice);
  m_cubemapConverter = std::make_unique<gfx::CubemapConverter>(rd);
  m_renderer = std::make_unique<gfx::WorldRenderer>(*m_cubemapConverter);

  m_luaState = createLuaState();
  m_luaState["GameWindow"] = std::ref(getWindow());
  m_luaState["InputSystem"] = std::ref(getInputSystem());

  _setupWidgets();

  auto &sceneEditor = m_widgets.get<SceneEditor>();

  sceneEditor.on<SceneEditor::EditScriptRequest>(
    [this](SceneEditor::EditScriptRequest &evt, auto &) {
      m_widgets.getConfig<ScriptEditor>().open = true;
      m_widgets.get<ScriptEditor>().open(evt.resource->getPath());
    });

  m_widgets.get<ScriptEditor>().on<ScriptEditor::RunScriptRequest>(
    [this](ScriptEditor::RunScriptRequest &req, auto &) {
      const sol::environment *env{nullptr};
      if (auto *scene = m_widgets.get<SceneEditor>().getCurrentScene(); scene) {
        env = std::addressof(getScriptContext(scene->getRegistry()).defaultEnv);
      }
      std::thread{[this, env, code = std::move(req.code)]() {
        auto result =
          env ? m_luaState.safe_script(code, *env, &sol::script_pass_on_error)
              : m_luaState.safe_script(code, &sol::script_pass_on_error);
        if (!result.valid()) {
          const sol::error err = result;
          SPDLOG_ERROR(err.what());
        }
      }}.join();
    });

  if (args.size() > 1) {
    const auto projectPath = std::filesystem::current_path() / args[1];
    if (auto settings = load(projectPath); settings) {
      m_projectSettings.emplace(std::move(*settings));
    } else {
      SPDLOG_ERROR(settings.error());
    }
  }

  getWindow().maximize();
}
App::~App() {
  getRenderDevice().waitIdle();
  Services::reset();

  JoltPhysics::cleanup();
};

void App::_setupWidgets() {
#ifdef _DEBUG
  m_widgets.add<SimpleWidgetWindow>(
    "Demo", {.open = false},
    [](const char *, bool *open) { ImGui::ShowDemoWindow(open); });
#endif

  m_widgets.add<FileBrowserWidget>(
    "File Browser", {
                      .name = ICON_FA_FOLDER_TREE " File Browser",
                      .open = true,
                      .section = DockSpaceSection::Left,
                    });
  m_widgets.add<ResourcesWidget>("Resources",
                                 {
                                   .name = ICON_FA_WAREHOUSE " Resources",
                                   .open = true,
                                   .section = DockSpaceSection::LeftBottom,
                                 });

  m_widgets.add<LoggerWidget>("Logger",
                              {
                                .name = ICON_FA_MESSAGE " Logger",
                                .open = true,
                                .section = DockSpaceSection::Bottom,
                              },
                              *spdlog::default_logger());

  m_widgets.add<SimpleWidgetWindow>("GPU",
                                    {
                                      .name = ICON_FA_MICROCHIP " GPU",
                                      .open = true,
                                      .section = DockSpaceSection::BottomLeft,
                                    },
                                    [this](const char *name, bool *open) {
                                      showGPUWindow(name, open,
                                                    getRenderDevice());
                                    });
  m_widgets.add<SimpleWidgetWindow>(
    "World Renderer",
    {
      .name = ICON_FA_EARTH_EUROPE " World Renderer",
      .open = true,
      .section = DockSpaceSection::BottomLeft,
    },
    [this](const char *name, bool *open) {
      showWorldRendererWindow(name, open, *m_renderer);
    });
  m_widgets.add<ShapeCreatorWidget>("Shape Creator",
                                    {.name = "Shape Creator", .open = false});

  m_widgets.add<SceneEditor>("Scene Editor",
                             {
                               .name = ICON_FA_IMAGES " Scene Editor",
                               .open = true,
                               .section = DockSpaceSection::Center,
                             },
                             getInputSystem(), *m_renderer, *m_audioDevice,
                             m_luaState);
  m_widgets.add<ScriptEditor>("Script Editor",
                              {
                                .name = ICON_FA_CODE " Script Editor",
                                .open = false,
                                .section = DockSpaceSection::Center,
                              });
  m_widgets.add<MaterialEditor>("Material Editor",
                                {
                                  .name = ICON_FA_BRUSH " Material Editor",
                                  .open = false,
                                  .section = DockSpaceSection::Center,
                                },
                                getInputSystem(), *m_renderer);
}

void App::_onGUI() {
  ZoneScoped;

  const auto *mainViewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(mainViewport->WorkPos);
  ImGui::SetNextWindowSize(mainViewport->WorkSize);
  ImGui::SetNextWindowViewport(mainViewport->ID);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});

  constexpr auto kHostWindowFlags =
    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
    ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus |
    ImGuiWindowFlags_NoNavFocus;
  ImGui::Begin("Main", nullptr, kHostWindowFlags);
  ImGui::PopStyleVar(3);
  _mainMenuBar();
  const auto dockspaceId = ImGui::GetID("DockSpace");
  _setupDockSpace(dockspaceId);
  ImGui::DockSpace(dockspaceId);

#if _DEBUG
  {
    static auto debugBuildWarning{true};

    constexpr auto kDebugBuildWarningId = MAKE_WARNING("Warning");
    if (debugBuildWarning) ImGui::OpenPopup(kDebugBuildWarningId);

    if (const auto button =
          showMessageBox(kDebugBuildWarningId,
                         "You are running the editor with the DEBUG option.\n"
                         "This option results in slower execution.");
        button == ModalButton::Ok) {
      debugBuildWarning = false;
    }
  }
#endif

  if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
    ImGui::OpenPopup(GUI::Modals::kCloseEditorId,
                     ImGuiPopupFlags_NoOpenOverExistingPopup);
  }
  if (const auto button =
        showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
          GUI::Modals::kCloseEditorId, "Close the editor?");
      button == ModalButton::Yes) {
    close();
  }

  // ---

  const auto cleanupLastProject = [this] {
    m_widgets.get<SceneEditor>().closeAllScenes();
    m_widgets.get<MaterialEditor>().clear();

    Services::Resources::clear();
  };

  constexpr auto kProjectExtension = ".project";
  constexpr auto projectFilter = makeExtensionFilter(kProjectExtension);

  static auto rootPath = std::filesystem::current_path();
  if (const auto p =
        showFileDialog(GUI::Modals::kNewProjectId,
                       {
                         .dir = rootPath,
                         .entryFilter = projectFilter,
                         .forceExtension = kProjectExtension,
                         .flags = FileDialogFlags_AskOverwrite |
                                  FileDialogFlags_CreateDirectoryButton,
                       });
      p) {
    if (auto settings = createProject(*p)) {
      m_projectSettings.emplace(std::move(*settings));
      cleanupLastProject();
    } else {
      SPDLOG_ERROR(settings.error());
    }
  }
  if (const auto p = showFileDialog(GUI::Modals::kOpenProjectId,
                                    {
                                      .dir = rootPath,
                                      .entryFilter = projectFilter,
                                    });
      p) {
    if (auto settings = load(*p)) {
      m_projectSettings.emplace(std::move(*settings));
      cleanupLastProject();
    } else {
      SPDLOG_ERROR(settings.error());
    }
  }
  if (m_projectSettings) {
    showProjectSettings(GUI::Modals::kProjectSettingsId, *m_projectSettings);
  }

  if (const auto p =
        showFileDialog(GUI::Modals::kSaveThemeId,
                       {
                         .dir = rootPath,
                         .forceExtension = ".json",
                         .flags = FileDialogFlags_AskOverwrite |
                                  FileDialogFlags_CreateDirectoryButton,
                       });
      p) {
    save(*p, ImGui::GetStyle());
  }
  if (const auto p =
        showFileDialog(GUI::Modals::kLoadThemeId,
                       {
                         .dir = rootPath,
                         .entryFilter = makeExtensionFilter(".json"),
                       });
      p) {
    load(*p, ImGui::GetStyle());
  }

  if (const auto button =
        showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
          GUI::Modals::kCloseProjectId, "Close the project?");
      button == ModalButton::Yes) {
    m_projectSettings = std::nullopt;
    cleanupLastProject();
  }
  ImGui::End();

  if (m_projectSettings) ::show(m_widgets);
}

void App::_setupDockSpace(const ImGuiID dockspaceId) const {
  if (static auto firstTime = true; firstTime) [[unlikely]] {
    firstTime = false;

    if (ImGui::DockBuilderGetNode(dockspaceId) == nullptr) {
      ImGui::DockBuilderRemoveNode(dockspaceId);

      ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
      ImGui::DockBuilderSetNodeSize(dockspaceId,
                                    ImGui::GetMainViewport()->Size);

      auto centerNodeId = dockspaceId;

      auto leftNodeId = ImGui::DockBuilderSplitNode(
        centerNodeId, ImGuiDir_Left, 0.15f, nullptr, &centerNodeId);
      const auto leftBottomNodeId = ImGui::DockBuilderSplitNode(
        leftNodeId, ImGuiDir_Down, 0.35f, nullptr, &leftNodeId);
      auto bottomNodeId = ImGui::DockBuilderSplitNode(
        centerNodeId, ImGuiDir_Down, 0.2f, nullptr, &centerNodeId);
      const auto bottomLeftNodeId = ImGui::DockBuilderSplitNode(
        bottomNodeId, ImGuiDir_Left, 0.15f, nullptr, &bottomNodeId);

      const auto getSectionId = [=](std::optional<DockSpaceSection> in) {
        return map(in, [=](DockSpaceSection section) -> ImGuiID {
          switch (section) {
            using enum DockSpaceSection;

          case Left:
            return leftNodeId;
          case LeftBottom:
            return leftBottomNodeId;
          case Center:
            return centerNodeId;
          case Bottom:
            return bottomNodeId;
          case BottomLeft:
            return bottomLeftNodeId;
          }
          assert(false);
          return 0;
        });
      };

      for (const auto &[_, window] : m_widgets) {
        const auto &config = window.config;
        if (const auto nodeId = getSectionId(config.section); nodeId) {
          ImGui::DockBuilderDockWindow(config.name, *nodeId);
        }
      }

      ImGui::DockBuilderFinish(dockspaceId);
    }
  }
}
void App::_mainMenuBar() {
  std::optional<const char *> action;
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Editor")) {
      if (ImGui::BeginMenu("Theme")) {
        if (ImGui::MenuItem("Save")) {
          action = GUI::Modals::kSaveThemeId;
        }
        if (ImGui::MenuItem("Load")) {
          action = GUI::Modals::kLoadThemeId;
        }
        ImGui::EndMenu();
      }
      ImGui::Separator();
      if (ImGui::MenuItemEx("Close", ICON_FA_XMARK, "ESC")) {
        action = GUI::Modals::kCloseEditorId;
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Project")) {
      const auto hasProject = m_projectSettings.has_value();
      if (ImGui::MenuItemEx("New", ICON_FA_FILE)) {
        action = GUI::Modals::kNewProjectId;
      }
      if (ImGui::MenuItemEx("Save", ICON_FA_DOWNLOAD, nullptr, false,
                            hasProject)) {
        save(*m_projectSettings);
      }
      if (ImGui::MenuItemEx("Load", ICON_FA_UPLOAD)) {
        action = GUI::Modals::kOpenProjectId;
      }

      ImGui::Separator();

      if (ImGui::MenuItemEx("Settings", ICON_FA_GEAR, nullptr, false,
                            hasProject)) {
        action = GUI::Modals::kProjectSettingsId;
      }

      ImGui::Separator();

      if (ImGui::MenuItemEx("Close", ICON_FA_XMARK, nullptr, false,
                            hasProject)) {
        action = GUI::Modals::kCloseProjectId;
      }

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu(ICON_FA_SCREWDRIVER_WRENCH " Tools")) {
      ImGui::MenuItems(m_widgets);
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("RenderDoc", hasRenderDoc())) {
      if (ImGui::MenuItem("Capture")) {
        captureFrame();
      }
      ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
  }

  if (action) ImGui::OpenPopup(*action);
}

#define INVOKE(widgets, func, ...)                                             \
  for (const auto &[_, entry] : widgets) {                                     \
    if (entry.config.open) entry.widget->func(__VA_ARGS__);                    \
  }

void App::_onInput(const os::InputEvent &evt) {
  ImGuiApp::_onInput(evt);
  INVOKE(m_widgets, onInput, evt)
}

void App::_onUpdate(fsec dt) {
  ImGuiApp::_onUpdate(dt);
  INVOKE(m_widgets, onUpdate, dt.count())
}
void App::_onPhysicsUpdate(fsec dt) {
  INVOKE(m_widgets, onPhysicsUpdate, dt.count())
}

void App::_onPreRender() {
  ImGui::NewFrame();
  _onGUI();
  {
    ZoneScopedN("ImGui::Render");
    ImGui::Render();
  }
}
void App::_onRender(rhi::CommandBuffer &cb, rhi::RenderTargetView rtv,
                    fsec dt) {
  INVOKE(m_widgets, onRender, cb, dt.count())
  drawGui(cb, rtv);
}
