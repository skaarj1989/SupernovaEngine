#include "MaterialEditor/MaterialEditor.hpp"

#include "TypeTraits.hpp"
#include "AlwaysFalse.hpp"

#include "MaterialEditor/LuaMaterialEditor.hpp"
#include "GraphCommands.hpp"
#include "ProjectCommands.hpp"

#include "ImGuiTitleBarMacro.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "ImGuiHelper.hpp"
#include "ImGuiModal.hpp"
#include "FileDialog.hpp"
#include "TextEditorCommon.hpp"

#include "imgui_internal.h" // DockBuilder*, *Ex,  Push/PopItemFlag
#include "imgui_stdlib.h"   // InputText(WithHint)

#include "spdlog/spdlog.h"

#include <ranges>
#include <algorithm>

namespace {

constexpr auto kProjectExtension = ".mgproj";

struct GUI {
  GUI() = delete;

  struct Windows {
    Windows() = delete;

#define TOOL_ID "##MaterialEditor"
    static constexpr auto kNodeCanvas =
      ICON_FA_DIAGRAM_PROJECT " NodeCanvas" TOOL_ID;
    static constexpr auto kCodeEditor = ICON_FA_CODE " Code" TOOL_ID;
    static constexpr auto kPreview = ICON_FA_TV " Preview" TOOL_ID;
    static constexpr auto kRenderSettings =
      ICON_FA_GEARS " RenderSettings" TOOL_ID;
    static constexpr auto kSceneSettings = ICON_FA_IMAGE " Scene" TOOL_ID;
    static constexpr auto kSettings = ICON_FA_SLIDERS " Settings" TOOL_ID;
    static constexpr auto kNodeList = ICON_FA_LIST " Nodes" TOOL_ID;
#undef TOOL_ID
  };
  struct Actions {
    static constexpr auto kLoadMaterial =
      MAKE_TITLE_BAR(ICON_FA_UPLOAD, "Open Material");
    static constexpr auto kSaveMaterialAs =
      MAKE_TITLE_BAR(ICON_FA_FLOPPY_DISK, "Save Material As...");
    static constexpr auto kExportMaterial =
      MAKE_TITLE_BAR(ICON_FA_FILE_EXPORT, "Export Material");
    static constexpr auto kDiscardMaterial = MAKE_WARNING("Discard Material?");

    static constexpr auto kShowKeyboardShortcuts =
      MAKE_TITLE_BAR(ICON_FA_CIRCLE_QUESTION, "Help");
  };
};

const std::vector<EditorActionInfo> kNodeCanvasActions{
  {"Define UserFunction", {"Ctrl+M"}},
  {"Edit UserFunction(s)", {"Ctrl+Alt+M"}},

  {"Select all", {"Ctrl+A"}},

  {"Undo", {"Ctrl+Z"}},
  {"Redo", {"Ctrl+Y"}},
  {"Graphviz (clipboard)", {"Ctrl+D"}},
  {"Graphviz (file)", {"Ctrl+Alt+D"}},
};
const std::vector<EditorActionInfo> kMaterialEditorActions{
  {"New Material (Surface)", {"Ctrl+N"}},
  {"New Material (PostProcess)", {"Ctrl+Alt+N"}},
  {"Open", {"Ctrl+O"}},
  {"Save", {"Ctrl+S"}},
  {"Save As...", {"Ctrl+Shift+S"}},
  {"Export", {"Ctrl+E"}},
  {"Go to VertexStage", {"Ctrl+F1"}},
  {"Go to FragmentStage", {"Ctrl+F2"}},
  {"Compose", {"F7"}},
  {"Build", {"Ctrl+B"}},
};

void listFiles(const std::filesystem::path &dir,
               std::vector<std::filesystem::path> &out) {
  for (auto &&entry : std::filesystem::directory_iterator{dir}) {
    if (entry.is_directory()) {
      listFiles(entry.path(), out);
    } else if (entry.is_regular_file()) {
      out.emplace_back(entry.path());
    }
  }
}

void stageMenuItems(rhi::ShaderType &v, const gfx::MaterialDomain domain) {
  using enum rhi::ShaderType;
  for (const auto [shaderType, shortcut] : {
         std::pair{Vertex, "Ctrl+F1"},
         std::pair{Fragment, "Ctrl+F2"},
       }) {
    const auto isSelected = (v == shaderType);
    const auto enabled =
      !(shaderType == Vertex && domain == gfx::MaterialDomain::PostProcess);
    if (ImGui::MenuItem(toString(shaderType), shortcut, isSelected, enabled)) {
      v = shaderType;
    }
  }
}

bool inspect(gfx::ShadingModel &shadingModel) {
  return ImGui::ComboEx("ShadingModel", shadingModel,
                        "Unlit\0"
                        "Lit\0"
                        "\0");
}
bool inspect(const char *label, gfx::BlendOp &blendOp) {
  return ImGui::ComboEx(label, blendOp,
                        "Keep\0"
                        "Replace\0"
                        "Blend\0"
                        "\0");
}
bool inspect(gfx::BlendMode &blendMode) {
  return ImGui::ComboEx("BlendMode", blendMode,
                        "Opaque\0"
                        "Masked\0"
                        "Transparent\0"
                        "Add\0"
                        "Modulate\0"
                        "\0");
}
bool inspect(gfx::LightingMode &lightingMode) {
  return ImGui::ComboEx("LightingMode", lightingMode,
                        "Default\0"
                        "Transmission\0"
                        "\0");
}
bool inspect(rhi::CullMode &cullMode) {
  return ImGui::ComboEx("CullMode", cullMode,
                        "None\0"
                        "Front\0"
                        "Back\0"
                        "\0");
}

bool inspect(gfx::Material::Blueprint &blueprint) {
  auto changed = false;
  if (blueprint.surface) {
    ImGui::BulletText("Domain: Surface");
    ImGui::Dummy({0, 2});

    auto &surface = *blueprint.surface;
    changed |= inspect(surface.shadingModel);

    if (ImGui::TreeNode("DecalBlendMode")) {
      auto &decalBlendMode = surface.decalBlendMode;
      changed |= inspect("Normal", decalBlendMode.normal);
      changed |= inspect("Emissive", decalBlendMode.emissive);
      changed |= inspect("Albedo", decalBlendMode.albedo);
      changed |=
        inspect("MetallicRoughnessAO", decalBlendMode.metallicRoughnessAO);

      ImGui::TreePop();
    }

    changed |= inspect(surface.blendMode);
    changed |= inspect(surface.lightingMode);
    changed |= inspect(surface.cullMode);

    // Change of flags should not trigger shader recompilation.

    ImGui::SeparatorText("Flags");
    ImGui::CheckboxFlags("CastShadow", blueprint.flags,
                         gfx::MaterialFlags::CastShadow);
    ImGui::CheckboxFlags("ReceiveShadow", blueprint.flags,
                         gfx::MaterialFlags::ReceiveShadow);
  } else {
    ImGui::BulletText("Domain: Postprocess");
    ImGui::Dummy({0, 2});

    ImGui::SeparatorText("Flags");
    ImGui::CheckboxFlags("Enabled", blueprint.flags,
                         gfx::MaterialFlags::Enabled);
  }
  return changed;
}

} // namespace

//
// MaterialEditor class:
//

MaterialEditor::MaterialEditor(os::InputSystem &inputSystem,
                               gfx::WorldRenderer &renderer)
    : m_logger{spdlog::default_logger()}, m_project{&m_scriptedFunctions},
      m_nodeEditor{m_nodeFactoryRegistry},
      m_previewWidget{inputSystem, renderer} {
  m_logger->set_level(spdlog::level::trace);

  m_luaState.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math);
  registerMaterialNodes(m_luaState);
  _loadFunctions();

  m_nodeFactoryRegistry.load(m_scriptedFunctions);
  m_nodeEditor.refreshFunctionEntries(m_scriptedFunctions);

  _connectProject();
  m_project.init(gfx::MaterialDomain::Surface);
}
MaterialEditor::~MaterialEditor() { clear(); }

void MaterialEditor::clear() { m_project.clear(); }

void MaterialEditor::show(const char *name, bool *open) {
  ZoneScopedN("MaterialEditor");

  using OptionalAction = std::optional<const char *>;
  static std::pair<OptionalAction, OptionalAction> actions;
  static std::unique_ptr<Command> deferredCommand;

  const auto newMaterial = [this](const gfx::MaterialDomain materialDomain) {
    auto cmd = std::make_unique<NewMaterialCommand>(m_project, materialDomain);
    if (m_project.isChanged()) {
      actions.first = GUI::Actions::kDiscardMaterial;
      deferredCommand = std::move(cmd);
    } else {
      m_project.addCommand(std::move(cmd));
    }
  };
  const auto saveProject = [&] {
    if (m_project.isOnDisk()) {
      if (auto error = m_project.save(); !error) {
        m_logger->info("MaterialProject saved.");
      } else {
        m_logger->error("Failed to save MaterialProject. {}", *error);
      }
    } else {
      actions.first = GUI::Actions::kSaveMaterialAs;
    }
  };
  const auto loadProject = [&] {
    if (m_project.isChanged()) {
      actions = {
        GUI::Actions::kDiscardMaterial,
        GUI::Actions::kLoadMaterial,
      };
    } else {
      actions.first = GUI::Actions::kLoadMaterial;
    }
  };
  const auto composeCurrentStage = [this]() {
    m_project.addCommand<ComposeShaderCommand>(m_project,
                                               rhi::getStage(m_shaderType));
  };
  const auto buildMaterial = [this] {
    m_project.addCommand<BuildMaterialCommand>(m_project,
                                               m_previewWidget.getRenderer());
  };
  const auto exportMaterial = [&] {
    if (m_project.isOnDisk()) {
      if (auto error = m_project.exportMaterial(); !error) {
        m_logger->info("Material exported.");
      } else {
        m_logger->error("Failed to export material. {}", *error);
      }
    } else {
      actions.first = GUI::Actions::kExportMaterial;
    }
  };

  ImGui::Begin(name, open,
               ImGuiWindowFlags_MenuBar |
                 (m_project.isChanged() ? ImGuiWindowFlags_UnsavedDocument
                                        : ImGuiWindowFlags_None));
  auto hasFocus = ImGui::IsWindowFocused();
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Material")) {
      if (ImGui::BeginMenu("New...")) {
        using enum gfx::MaterialDomain;
        if (ImGui::MenuItemEx("Surface", ICON_FA_CUBE, "Ctrl+N")) {
          newMaterial(Surface);
        }
        if (ImGui::MenuItemEx("PostProcess", ICON_FA_IMAGE, "Ctrl+Alt+N")) {
          newMaterial(PostProcess);
        }
        ImGui::EndMenu();
      }
      if (ImGui::MenuItemEx("Open", ICON_FA_FILE_ARROW_UP, "Ctrl+O")) {
        loadProject();
      }
      if (ImGui::MenuItemEx("Save", ICON_FA_FLOPPY_DISK, "Ctrl+S", false,
                            m_project)) {
        saveProject();
      }
      if (ImGui::MenuItemEx("Save As...", ICON_FA_FILE_ARROW_DOWN,
                            "Ctrl+Shift+S", false, m_project)) {
        actions.first = GUI::Actions::kSaveMaterialAs;
      }
      if (ImGui::MenuItemEx("Export", ICON_FA_FILE_EXPORT, "Ctrl+E", false,
                            m_project)) {
        exportMaterial();
      }

      ImGui::Separator();

      ImGui::BeginDisabled(!m_project);
      if (ImGui::MenuItem("Compose", "F7", false, !m_livePreview)) {
        composeCurrentStage();
      }
      if (ImGui::MenuItem("Compile", "Ctrl+B")) {
        buildMaterial();
      }
      ImGui::Checkbox("Live preview", &m_livePreview);
      ImGui::EndDisabled();

      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Stage", m_project)) {
      stageMenuItems(m_shaderType, m_project.getMaterialDomain());
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Windows")) {
      ImGui::MenuItem("Canvas", nullptr, &m_windows.canvas);
      ImGui::MenuItem("Node List", nullptr, &m_windows.nodeList);
      ImGui::MenuItem("Code Editor", nullptr, &m_windows.codeEditor);
      ImGui::MenuItem("Settings", nullptr, &m_windows.settings);
      ImGui::MenuItem("Scene Settings", nullptr, &m_windows.sceneSettings);
      ImGui::MenuItem("Render Settings", nullptr, &m_windows.renderSettings);
      ImGui::MenuItem("Preview", nullptr, &m_windows.preview);

      ImGui::EndMenu();
    }

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 2.0f);

    if (ImGui::BeginMenu("Help")) {
      if (ImGui::MenuItem("Keyboard shortcuts", "Ctrl+H")) {
        actions.first = GUI::Actions::kShowKeyboardShortcuts;
      }
      ImGui::EndMenu();
    }

    ImGui::EndMenuBar();
  }

  const auto dockspaceId = ImGui::GetID("DockSpace");
  _setupDockSpace(dockspaceId);
  ImGui::DockSpace(dockspaceId);
  onDropTarget(kImGuiPayloadTypeFile, [this](const auto *payload) {
    ImGui::SetWindowFocus();

    auto p = ImGui::ExtractPath(*payload);
    if (const auto ext = os::FileSystem::getExtension(p);
        ext == kProjectExtension) {
      if (m_project != p) {
        auto cmd =
          std::make_unique<LoadProjectCommand>(m_project, std::move(p));
        if (m_project.isChanged()) {
          actions.first = GUI::Actions::kDiscardMaterial;
          deferredCommand = std::move(cmd);
        } else {
          m_project.addCommand(std::move(cmd));
        }
      }
    } else {
      m_logger->error("Invalid extension: '{}', expected: '{}'",
                      ext.value_or(""), kProjectExtension);
    }
  });
  ImGui::End();

  if (m_shaderType == rhi::ShaderType::Vertex &&
      m_project.getMaterialDomain() == gfx::MaterialDomain::PostProcess) {
    m_shaderType = rhi::ShaderType::Fragment;
  }

  auto *stage = m_project.getStage(m_shaderType);

  if (m_windows.canvas) {
    if (ImGui::Begin(GUI::Windows::kNodeCanvas, &m_windows.canvas,
                     ImGuiWindowFlags_MenuBar)) {
      hasFocus |= ImGui::IsWindowFocused(ImGuiHoveredFlags_ChildWindows);
      m_nodeEditor.showCanvas(m_project, makeView(*stage), *m_logger);
    }
    ImGui::End();
  }
  if (m_windows.nodeList) {
    if (ImGui::Begin(GUI::Windows::kNodeList, &m_windows.nodeList)) {
      hasFocus |= ImGui::IsWindowFocused();
      m_nodeEditor.showNodeList(makeView(*stage));
    }
    ImGui::End();
  }
  if (m_windows.codeEditor) {
    if (ImGui::Begin(GUI::Windows::kCodeEditor, &m_windows.codeEditor)) {
      ZoneScopedN("MaterialEditor::CodeEditor");
      hasFocus |= ImGui::IsWindowFocused();
      basicTextEditorWidget(stage->codeEditor, IM_UNIQUE_ID);
    }
    ImGui::End();
  }
  if (m_windows.settings) {
    if (ImGui::Begin(GUI::Windows::kSettings, &m_windows.settings)) {
      ZoneScopedN("MaterialEditor::Settings");
      hasFocus |= ImGui::IsWindowFocused();
      _settingsWidget();
    }
    ImGui::End();
  }

  if (m_windows.sceneSettings) {
    m_previewWidget.showSceneSettings(GUI::Windows::kSceneSettings,
                                      &m_windows.sceneSettings);
  }
  if (m_windows.renderSettings) {
    m_previewWidget.showRenderSettings(GUI::Windows::kRenderSettings,
                                       &m_windows.renderSettings);
  }
  if (m_windows.preview) {
    m_previewWidget.showPreview(GUI::Windows::kPreview, &m_windows.preview);
  }

  // --- Keyboard shortcuts (main):

  if (hasFocus && !actions.first) {
    using enum gfx::MaterialDomain;
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_H)) {
      actions.first = GUI::Actions::kShowKeyboardShortcuts;
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_N)) {
      newMaterial(Surface);
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Alt |
                                        ImGuiKey_N)) {
      newMaterial(PostProcess);
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_O)) {
      loadProject();
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S)) {
      saveProject();
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift |
                                        ImGuiKey_S)) {
      actions.first = GUI::Actions::kSaveMaterialAs;
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_E)) {
      exportMaterial();
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_F1)) {
      m_shaderType = rhi::ShaderType::Vertex;
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_F2)) {
      m_shaderType = rhi::ShaderType::Fragment;
    } else if (ImGui::IsKeyChordPressed(ImGuiKey_F7)) {
      composeCurrentStage();
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_B)) {
      buildMaterial();
    }
  }

  if (actions.first) {
    ImGui::OpenPopup(*actions.first);
    actions.first.reset();
  }

  showModal<ModalButtons::Ok>(GUI::Actions::kShowKeyboardShortcuts, [] {
    keyboardShortcutsTable("Material Editor", kMaterialEditorActions);
    keyboardShortcutsTable("Node Editor", kNodeCanvasActions);
  });

  if (const auto button =
        showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
          GUI::Actions::kDiscardMaterial,
          "Do you really want to discard changes?");
      button) {
    if (button == ModalButton::Yes) {
      actions = {actions.second, std::nullopt};
      if (deferredCommand) {
        m_project.addCommand(std::move(deferredCommand));
      }
    } else {
      deferredCommand.reset();
      actions = {};
    }
  }

  static const auto projectFilter = makeExtensionFilter(kProjectExtension);

  const auto &rootDir = os::FileSystem::getRoot();
  static auto currentDir = rootDir;

  if (const auto p = showFileDialog(GUI::Actions::kLoadMaterial,
                                    {
                                      .dir = currentDir,
                                      .barrier = rootDir,
                                      .entryFilter = projectFilter,
                                    });
      p) {
    m_project.addCommand<LoadProjectCommand>(m_project, *p);
  }
  if (const auto p =
        showFileDialog(GUI::Actions::kSaveMaterialAs,
                       {
                         .dir = currentDir,
                         .barrier = rootDir,
                         .entryFilter = projectFilter,
                         .forceExtension = kProjectExtension,
                         .flags = FileDialogFlags_AskOverwrite |
                                  FileDialogFlags_CreateDirectoryButton,
                       });
      p) {
    m_project.addCommand<SaveProjectCommand>(m_project, *p);
  }
  if (const auto p =
        showFileDialog(GUI::Actions::kExportMaterial,
                       {
                         .dir = currentDir,
                         .barrier = rootDir,
                         .forceExtension = ".material",
                         .flags = FileDialogFlags_AskOverwrite |
                                  FileDialogFlags_CreateDirectoryButton,
                       });
      p) {
    m_project.addCommand<ExportMaterialCommand>(m_project, *p);
  }

  if (const auto count = m_project.executeAll(Command::Context{*m_logger});
      count > 0) {
    m_logger->trace("[CommandInvoker] Executed: {} command(s).", count);
  }
}

void MaterialEditor::onRender(rhi::CommandBuffer &cb, float dt) {
  ZoneScopedN("MaterialEditor::OnRender");
  m_previewWidget.onRender(cb, dt);
}

//
// (private):
//

void MaterialEditor::_loadFunctions() {
  std::vector<std::filesystem::path> scriptPaths;
  scriptPaths.reserve(100);
  listFiles("./assets/MaterialEditor/nodes", scriptPaths);

  for (const auto &p : scriptPaths) {
    if (auto data = loadFunction(p, m_luaState); data) {
      auto scriptedFunction = std::move(*data);
      if (!m_scriptedFunctions.contains(scriptedFunction.id)) {
        m_scriptedFunctions[scriptedFunction.id] =
          std::make_unique<ScriptedFunction::Data>(
            std::move(scriptedFunction.data));
      }
    } else {
      m_logger->error(data.error());
    }
  }
}

void MaterialEditor::_connectProject() {
  m_project.on<MaterialProjectInitializedEvent>(
    [this](const auto &, const auto &project) {
      m_logger->trace("[Event] MaterialProjectInitialized");

      const auto &userFunctions = project.getUserFunctions();
      if (m_nodeFactoryRegistry.load(userFunctions) > 0) {
        m_nodeEditor.refreshFunctionEntries(userFunctions);
      }
    });
  m_project.on<BeforeMaterialProjectUnloadEvent>(
    [this](const auto &, const auto &project) {
      m_logger->trace("[Event] BeforeMaterialProjectUnload");

      const auto &userFunctions = project.getUserFunctions();
      if (m_nodeFactoryRegistry.unload(userFunctions) > 0) {
        m_nodeEditor.purgeUserFunctionMenu();
      }
    });

  m_project.on<MasterNodeChangedEvent>(
    [this](const MasterNodeChangedEvent &evt, const auto &) {
      m_logger->trace("[Event] MasterNodeChanged");
      if (m_livePreview) {
        if (m_project.hasDirtyStages()) {
          m_logger->trace(" ComposeShaderRequest");
          m_project.addCommand<ComposeShaderCommand>(m_project, evt.stages);
        }
      }
    });

  m_project.on<UserFunctionAddedEvent>(
    [this](const UserFunctionAddedEvent &evt, const auto &project) {
      m_nodeFactoryRegistry.add({evt.id, project.getUserFunctionData(evt.id)});
      m_nodeEditor.refreshFunctionEntries(project.getUserFunctions());
      m_logger->info("UserFunction[{}] Added", evt.id);
    });
  m_project.on<UserFunctionUpdatedEvent>(
    [this](const UserFunctionUpdatedEvent &evt, const auto &) {
      m_logger->info("UserFunction[{}] Updated", evt.id);
    });
  m_project.on<UserFunctionRemovedEvent>(
    [this](const UserFunctionRemovedEvent &evt, const auto &project) {
      m_nodeFactoryRegistry.remove(evt.id);
      m_nodeEditor.refreshFunctionEntries(project.getUserFunctions());
      m_logger->info("UserFunction[{}] Removed", evt.id);
    });

  m_project.on<ShaderComposedEvent>(
    [this](const ShaderComposedEvent &evt, MaterialProject &project) {
      m_logger->trace("[Event] ShaderComposed");
      if (!project.hasDirtyStages()) {
        m_logger->trace("No dirty stages, building ...");
        project.addCommand<BuildMaterialCommand>(project,
                                                 m_previewWidget.getRenderer());
      }
    });

  m_project.on<MaterialBuiltEvent>(
    [this](MaterialBuiltEvent &evt, MaterialProject &project) {
      m_logger->trace("[Event] MaterialBuilt");
      m_previewWidget.updateMaterial(std::move(evt.material));
    });
}

void MaterialEditor::_setupDockSpace(const ImGuiID dockspaceId) const {
  if (static auto firstTime = true; firstTime) [[unlikely]] {
    firstTime = false;

    if (ImGui::DockBuilderGetNode(dockspaceId) == nullptr) {
      ImGui::DockBuilderRemoveNode(dockspaceId);
      ImGui::DockBuilderAddNode(dockspaceId);

      auto centerNodeId = dockspaceId;

      auto leftNodeId = ImGui::DockBuilderSplitNode(
        centerNodeId, ImGuiDir_Left, 0.21f, nullptr, &centerNodeId);
      auto leftMidNodeId = ImGui::DockBuilderSplitNode(
        leftNodeId, ImGuiDir_Down, 0.65f, nullptr, &leftNodeId);
      auto leftBottomNodeId = ImGui::DockBuilderSplitNode(
        leftMidNodeId, ImGuiDir_Down, 0.67f, nullptr, &leftMidNodeId);

      auto rightNodeId = ImGui::DockBuilderSplitNode(
        centerNodeId, ImGuiDir_Right, 0.26f, nullptr, &centerNodeId);
      auto rightBottomNodeId = ImGui::DockBuilderSplitNode(
        rightNodeId, ImGuiDir_Down, 0.6f, nullptr, &rightNodeId);

      ImGui::DockBuilderDockWindow(GUI::Windows::kPreview, leftNodeId);
      ImGui::DockBuilderDockWindow(GUI::Windows::kCodeEditor,
                                   rightBottomNodeId);
      ImGui::DockBuilderDockWindows(
        {
          GUI::Windows::kSettings,
        },
        leftMidNodeId);
      ImGui::DockBuilderDockWindow(GUI::Windows::kNodeList, rightNodeId);
      ImGui::DockBuilderDockWindows(
        {
          GUI::Windows::kRenderSettings,
          GUI::Windows::kSceneSettings,
        },
        leftBottomNodeId);
      ImGui::DockBuilderDockWindow(GUI::Windows::kNodeCanvas, centerNodeId);

      ImGui::DockBuilderFinish(dockspaceId);
    }
  }
}

void MaterialEditor::_settingsWidget() {
  ImGui::SetNextItemWidth(150);
  ImGui::InputText("Name", &m_project.m_name);

  ImGui::Dummy({0, 2});
  ImGui::PushItemWidth(100);
  if (inspect(m_project.m_blueprint)) {
    m_project.addCommand<BuildMaterialCommand>(m_project,
                                               m_previewWidget.getRenderer());
  }
  ImGui::PopItemWidth();
}
