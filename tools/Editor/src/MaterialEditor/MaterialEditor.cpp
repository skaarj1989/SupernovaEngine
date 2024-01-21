#include "MaterialEditor/MaterialEditor.hpp"
#include "MaterialEditor/LuaMaterialEditor.hpp"

#include "TypeTraits.hpp"
#include "AlwaysFalse.hpp"

#include "Services.hpp"

#include "ImNodesStyleWidget.hpp"
#include "ImNodesStyleJSON.hpp"

#include "NodeContextMenuEntries.hpp"

#include "ImGuiDragAndDrop.hpp"
#include "MaterialEditor/ImNodesHelper.hpp"
#include "ImGuiTitleBarMacro.hpp"
#include "InspectEnum.hpp"

#include "FileDialog.hpp"
#include "ImGuiPopups.hpp"
#include "ImGuiModal.hpp"
#include "imgui_internal.h"
#include "imgui_stdlib.h" // InputText(WithHint)

#include "spdlog/spdlog.h"
#include "spdlog/fmt/chrono.h"

#include <fstream> // ofstream
#include <ranges>

namespace {

#define TOOL_ID "##MaterialEditor"

struct GUI {
  GUI() = delete;

  struct Windows {
    Windows() = delete;

    static constexpr auto kCanvas = ICON_FA_DIAGRAM_PROJECT " Canvas" TOOL_ID;
    static constexpr auto kCodeEditor = ICON_FA_CODE " Code" TOOL_ID;
    static constexpr auto kPreview = ICON_FA_TV " Preview" TOOL_ID;
    static constexpr auto kRenderSettings = ICON_FA_GEARS " RenderSettings";
    static constexpr auto kSceneSettings = ICON_FA_IMAGE " Scene";
    static constexpr auto kSettings = ICON_FA_SLIDERS " Settings" TOOL_ID;
    static constexpr auto kNodeList = ICON_FA_LIST " Nodes" TOOL_ID;
    static constexpr auto kLogger = ICON_FA_MESSAGE " Logger" TOOL_ID;
  };

  struct Modals {
    Modals() = delete;
  };
};

#undef TOOL_ID

template <typename... _Ty, typename... _Types>
constexpr bool holds_any_of(std::variant<_Types...> const &v) noexcept {
  return (std::holds_alternative<_Ty>(v) || ...);
}

void stageSelector(const char *name, rhi::ShaderType &v, bool surface) {
  if (ImGui::BeginCombo(name, toString(v))) {
    using enum rhi::ShaderType;
    for (const auto shaderType : {Vertex, Fragment}) {
      const auto isSelected = (v == shaderType);
      const auto flags = !surface && shaderType == Vertex
                           ? ImGuiSelectableFlags_Disabled
                           : ImGuiSelectableFlags_None;
      if (ImGui::Selectable(toString(shaderType), isSelected, flags)) {
        v = shaderType;
      }
    }
    ImGui::EndCombo();
  }
}

struct Request {
  enum class Operation { Clone, Remove };
  Operation operation;
  VertexDescriptor vd{nullptr};
  ImVec2 position;
};
[[nodiscard]] bool
handleRequest(ShaderGraph &g, std::optional<Request> &request,
              const std::set<VertexDescriptor> &connectedVertices) {
  auto changed = false;

  const auto &[operation, vd, clickPos] = *request;
  switch (operation) {
    using enum Request::Operation;

  case Clone: {
    const auto [_, id] = clone(g, vd);
    ImNodes::ClearSelection();
    ImNodes::SetNodeScreenSpacePos(id, clickPos);
    ImNodes::SelectNode(id);
  } break;
  case Remove:
    removeNode(g, vd);
    changed |= connectedVertices.contains(vd);
    break;
  }

  request = std::nullopt;
  return changed;
}

[[nodiscard]] auto nodeWidget(ShaderGraph &g, VertexProp &vertexProp,
                              const gfx::Material::Blueprint &blueprint) {
  const auto id = vertexProp.id;

  ImNodes::BeginNode(id);
  const auto changed = std::visit(
    [&g, &blueprint, id, userLabel = getUserLabel(vertexProp)](auto &&arg) {
      using T = std::decay_t<decltype(arg)>;

      if constexpr (is_any_v<T, std::monostate, SplitVector, SplitMatrix>) {
        assert(false); // Should have the 'Internal' flag.
        return false;
      } else if constexpr (std::is_same_v<T, ContainerNode>) {
        return arg.inspect(g, id);
      } else if constexpr (is_any_v<T, ValueVariant, PropertyValue,
                                    TextureParam>) {
        return inspectNode(id, userLabel, arg);
      } else if constexpr (is_any_v<T, Attribute, FrameBlockMember,
                                    CameraBlockMember, BuiltInConstant,
                                    BuiltInSampler>) {
        inspectNode(id, userLabel, arg);
        return false;
      } else if constexpr (std::is_same_v<T, CompoundNodeVariant>) {
        return std::visit([&g, id](auto &v) { return v.inspect(g, id); }, arg);
      } else if constexpr (std::is_same_v<T, MasterNodeVariant>) {
        return std::visit(
          [&g, id, &blueprint](auto &v) {
            using U = std::decay_t<decltype(v)>;

            if constexpr (std::is_same_v<U, SurfaceMasterNode>) {
              return v.inspect(g, id, *blueprint.surface);
            } else {
              return v.inspect(g, id);
            }
          },
          arg);
      } else {
        static_assert(always_false_v<T>, "non-exhaustive visitor!");
      }
    },
    vertexProp.variant);
  ImNodes::EndNode();

  return changed;
}

auto nodeContextMenu(const char *name, ShaderGraph &g, VertexDescriptor vd,
                     std::optional<Request> &request) {
  auto dirty = false;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {8, 8});
  if (ImGui::BeginPopup(name, ImGuiWindowFlags_NoMove)) {
    auto &vertexProp = g.getVertexProp(vd);
    const auto id = vertexProp.id;
    const auto isMasterNode = id == ShaderGraph::kRootNodeId;
    ImGui::Text(ICON_FA_HASHTAG " Node ID: %d", id);
    if (!isMasterNode) {
      ImGui::SetNextItemWidth(100);
      dirty = ImGui::InputTextWithHint(IM_UNIQUE_ID, "Label", &vertexProp.label,
                                       ImGuiInputTextFlags_EnterReturnsTrue);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    using enum Request::Operation;
    if (ImGui::MenuItemEx("Clone", ICON_FA_CLONE, nullptr, false,
                          !isMasterNode)) {
      request = {
        .operation = Clone,
        .vd = vd,
        .position = ImGui::GetMousePos(),
      };
    }

    ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
    ImGui::MenuItemEx("Remove", ICON_FA_TRASH, nullptr, false, !isMasterNode);
    ImGui::PopItemFlag();

    attachPopup(nullptr, ImGuiMouseButton_Left, [&request, vd] {
      ImGui::Text("Do you really want to remove the selected node?");
      ImGui::Separator();
      if (ImGui::Button(ICON_FA_TRASH " Yes")) {
        request = {.operation = Remove, .vd = vd};
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_BAN " No")) ImGui::CloseCurrentPopup();
    });

    ImGui::EndPopup();
  }
  ImGui::PopStyleVar();

  return dirty;
}

bool validId(int32_t id) { return id >= 0; }

[[nodiscard]] bool
removeLink(ShaderGraph &g, const EdgeDescriptor &ed,
           const std::set<VertexDescriptor> &connectedVertices) {
  g.removeEdge(ed);
  const auto c = g.getConnection(ed);
  return connectedVertices.contains(c.from);
}

[[nodiscard]] bool
removeNode(ShaderGraph &g, VertexDescriptor vd,
           const std::set<VertexDescriptor> &connectedVertices) {
  removeNode(g, vd);
  return connectedVertices.contains(vd);
}

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
    : m_logger{std::make_shared<spdlog::logger>("MaterialEditor")},
      m_loggerWidget{*m_logger}, m_previewWidget{inputSystem, renderer} {
  spdlog::register_logger(m_logger);

  m_luaState.open_libraries(sol::lib::base, sol::lib::table, sol::lib::math);
  registerMaterialNodes(m_luaState);
  _loadFunctions();

  ImNodes::CreateContext();
  load("./assets/MaterialEditor/ImNodes.json", ImNodes::GetStyle());

#define SET_MODIFIER(Name, Key)                                                \
  ImNodes::GetIO().Name.Modifier = &ImGui::GetIO().Key

  SET_MODIFIER(LinkDetachWithModifierClick, KeyAlt);
  SET_MODIFIER(MultipleSelectModifier, KeyCtrl);
#undef SET_MODIFIER

  _initMaterial(gfx::MaterialDomain::Surface);
}
MaterialEditor::~MaterialEditor() {
  clear();
  ImNodes::DestroyContext();
}

void MaterialEditor::clear() { m_project.clear(); }

void MaterialEditor::show(const char *name, bool *open) {
  ImGui::Begin(name, open, ImGuiWindowFlags_MenuBar);
  auto changed = _menuBar();
  const auto dockspaceId = ImGui::GetID("DockSpace");
  _setupDockSpace(dockspaceId);
  ImGui::DockSpace(dockspaceId);
  ImGui::End();

  if (!m_project) return;

  auto &stage = m_project.stages[m_shaderType];
  ImNodes::EditorContextSet(stage.nodeEditorContext.get());
  auto &graph = stage.graph;

  if (ImGui::Begin(GUI::Windows::kNodeList, nullptr,
                   ImGuiWindowFlags_NoFocusOnAppearing)) {
    _nodeListWidget(graph);
  }
  ImGui::End();

  if (ImGui::Begin(GUI::Windows::kSettings)) {
    changed |= _settingsWidget();
  }
  ImGui::End();

  if (ImGui::Begin(GUI::Windows::kCodeEditor, nullptr,
                   ImGuiWindowFlags_NoFocusOnAppearing)) {
    stage.codeEditor.Render(IM_UNIQUE_ID);
  }
  ImGui::End();

  constexpr auto kLoadStyleActionId = MAKE_TITLE_BAR(ICON_FA_UPLOAD, "Load");
  constexpr auto kSaveStyleActionId =
    MAKE_TITLE_BAR(ICON_FA_FLOPPY_DISK, "Save As ...");

  std::optional<const char *> action;
  static bool showStyleEditor = false;

  if (ImGui::Begin(GUI::Windows::kCanvas, nullptr, ImGuiWindowFlags_MenuBar)) {
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("Debug")) {
        if (ImGui::BeginMenuEx("Graphviz", ICON_FA_DIAGRAM_PROJECT)) {
          if (ImGui::MenuItemEx("File", ICON_FA_FLOPPY_DISK)) {
            std::ofstream out{"ShaderGraph.dot"};
            graph.exportGraphviz(out);
          }
          if (ImGui::MenuItemEx("Clipboard", ICON_FA_CLIPBOARD)) {
            std::ostringstream oss;
            graph.exportGraphviz(oss);
            ImGui::SetClipboardText(oss.str().c_str());
          }
          ImGui::EndMenu();
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("MiniMap")) {
        ImGui::Checkbox("Enabled", &m_miniMap.enabled);
        ImGui::PushItemWidth(110);
        ImGui::BeginDisabled(!m_miniMap.enabled);
        ImGui::SliderFloat("Size", &m_miniMap.size, 0.01f, 0.5f, "%.3f",
                           ImGuiSliderFlags_AlwaysClamp);
        ImGui::Combo("Location", &m_miniMap.location,
                     "Bottom-Left\0"
                     "Bottom-Right\0"
                     "Top-Left\0"
                     "TopRight\0"
                     "\0");
        ImGui::EndDisabled();
        ImGui::PopItemWidth();

        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Style")) {
        if (ImGui::MenuItem("Load")) {
          action = kLoadStyleActionId;
        }
        if (ImGui::MenuItem("Save")) {
          action = kSaveStyleActionId;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Edit")) {
          showStyleEditor = true;
        }
        ImGui::EndMenu();
      }

      ImGui::Separator();

      ImGui::CheckboxFlags("Grid Snapping", &ImNodes::GetStyle().Flags,
                           ImNodesStyleFlags_GridSnapping);

      ImGui::EndMenuBar();
    }

    if (showStyleEditor) {
      if (ImGui::Begin(MAKE_TITLE_BAR(ICON_FA_PALETTE, "Style"),
                       &showStyleEditor)) {
        ImNodes::ShowStyleEditor();
      }
      ImGui::End();
    }

    if (action) ImGui::OpenPopup(*action, ImGuiWindowFlags_NoMove);

    constexpr auto kStyleExtension = ".json";
    constexpr auto styleFilter = makeExtensionFilter(kStyleExtension);

    static auto rootPath = std::filesystem::current_path();

    if (const auto p = showFileDialog(kLoadStyleActionId,
                                      {
                                        .dir = rootPath,
                                        .entryFilter = styleFilter,
                                      });
        p) {
      load(*p, ImNodes::GetStyle());
    }
    if (const auto p =
          showFileDialog(kSaveStyleActionId,
                         {
                           .dir = rootPath,
                           .entryFilter = styleFilter,
                           .forceExtension = kStyleExtension,
                           .flags = FileDialogFlags_AskOverwrite |
                                    FileDialogFlags_CreateDirectoryButton,
                         });
        p) {
      save(*p, ImNodes::GetStyle());
    }

    // ---

    const auto connectedVertices = findConnectedVertices(graph);
    changed |= _canvasWidget(graph, connectedVertices);
    onDropTarget(kImGuiPayloadTypeFile, [this](const auto *payload) {
      // NOTE: &stage and &graph becomes dangling after _loadProject.
      _loadProject(ImGui::ExtractPath(*payload));
    });
  }
  ImGui::End();

  m_previewWidget.showPreview(GUI::Windows::kPreview);
  m_previewWidget.showSceneSettings(GUI::Windows::kSceneSettings);
  m_previewWidget.showRenderSettings(GUI::Windows::kRenderSettings);

  m_loggerWidget.show(GUI::Windows::kLogger, nullptr);

  if (changed && m_livePreview) _composeMaterialAndUpdatePreview();
}

void MaterialEditor::onRender(rhi::CommandBuffer &cb, float dt) {
  m_previewWidget.onRender(cb, dt);
}

//
// (private):
//

void MaterialEditor::_loadFunctions() {
  std::vector<std::filesystem::path> scripts;
  scripts.reserve(100);
  listFiles("./assets/MaterialEditor/nodes", scripts);

  for (const auto &p : scripts) {
    if (auto data = loadFunction(p, m_luaState); data) {
      auto &[guid, function] = *data;
      if (!m_scriptedFunctions.contains(guid)) {
        m_scriptedFunctions[guid] =
          std::make_unique<ScriptedFunctionData>(std::move(function));
      }
    } else {
      m_logger->error(data.error());
    }
  }
}

void MaterialEditor::_initMaterial(gfx::MaterialDomain domain) {
  m_project.init(domain);
  // Make the canvas of fragment shader stage active.
  // The vertex stage is unavailable in a PostProcess material.
  m_shaderType = rhi::ShaderType::Fragment;
}

bool MaterialEditor::_loadProject(const std::filesystem::path &p) {
  if (m_project.path && m_project.path == p) return false;

  const auto relativePath = os::FileSystem::relativeToRoot(p)->generic_string();

  MaterialProject project;
  auto payload = project.load(p);
  if (!payload) {
    m_logger->error("'{}': Is not a material project.", relativePath);
    return false;
  }
  m_project = std::move(project);
  if (!m_project.postLoad(Services::Resources::Textures::value(),
                          payload->texturePaths, m_scriptedFunctions)) {
    m_logger->critical("Could not finalize (missing a texture or a function).");
    _initMaterial(gfx::MaterialDomain::Surface);
    return false;
  }
  m_logger->info("Material project loaded from: '{}'", relativePath);
  return _composeMaterialAndUpdatePreview();
}

bool MaterialEditor::_composeMaterial() {
  if (const auto ns = m_project.composeMaterial(); ns) {
    m_logger->info("Material composed in: {}.",
                   std::chrono::duration_cast<std::chrono::milliseconds>(*ns));
    return true;
  } else {
    m_logger->error(ns.error());
    return false;
  }
}
bool MaterialEditor::_composeMaterialAndUpdatePreview() {
  return _composeMaterial() && _forceUpdatePreview();
}
bool MaterialEditor::_forceUpdatePreview() {
  const auto begin = std::chrono::steady_clock::now();

  auto material = m_project.buildMaterial();
  if (const auto errorLog = m_previewWidget.getRenderer().isValid(material);
      errorLog) {
    m_project.setErrorMarkers(*errorLog);
    m_logger->error("Corrupted material.");
    return false;
  }

  const auto end = std::chrono::steady_clock::now();

  m_project.resetErrorMarkers();
  m_previewWidget.updateMaterial(
    std::make_shared<gfx::Material>(std::move(material)));

  m_logger->info(
    "Shaders compiled in: {}.",
    std::chrono::duration_cast<std::chrono::milliseconds>(end - begin));
  return true;
}

bool MaterialEditor::_menuBar() {
  constexpr auto kSaveMaterialActionId =
    MAKE_TITLE_BAR(ICON_FA_FLOPPY_DISK, "Save As ...");
  constexpr auto kLoadMaterialActionId =
    MAKE_TITLE_BAR(ICON_FA_UPLOAD, "Load Material");

  constexpr auto kCreateCustomNodeActionId =
    MAKE_TITLE_BAR(ICON_FA_PEN_TO_SQUARE, "Custom Node");
  constexpr auto kEditCustomNodesActionId =
    MAKE_TITLE_BAR(ICON_FA_PENCIL, "Custom Node");

  const auto &rootDir = os::FileSystem::getRoot();
  static auto currentDir = rootDir;

  std::optional<const char *> action;
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Material")) {
      if (ImGui::BeginMenu("New ...")) {
        using enum gfx::MaterialDomain;
        if (ImGui::MenuItemEx("Surface", ICON_FA_CUBE)) {
          _initMaterial(Surface);
          _composeMaterialAndUpdatePreview();
        }
        if (ImGui::MenuItemEx("PostProcess", ICON_FA_IMAGE)) {
          _initMaterial(PostProcess);
          _composeMaterialAndUpdatePreview();
        }
        ImGui::EndMenu();
      }

      if (ImGui::MenuItemEx("Save", ICON_FA_FLOPPY_DISK, nullptr, false,
                            m_project)) {
        if (m_project.path) {
          m_project.save();
        } else {
          action = kSaveMaterialActionId;
        }
      }
      if (ImGui::MenuItemEx("Save As ...", ICON_FA_FILE_ARROW_DOWN, nullptr,
                            false, m_project)) {
        action = kSaveMaterialActionId;
      }
      if (ImGui::MenuItemEx("Load", ICON_FA_FILE_ARROW_UP))
        action = kLoadMaterialActionId;
      if (ImGui::MenuItemEx("Export", ICON_FA_FILE_EXPORT, nullptr, false,
                            m_project)) {
        if (m_project.path) {
          m_project.exportMaterial(*m_project.path);
        } else {
          action = kSaveMaterialActionId;
        }
      }

      ImGui::Separator();

      if (ImGui::BeginMenu("Custom node", m_project)) {
        if (ImGui::MenuItem("Define")) action = kCreateCustomNodeActionId;
        if (ImGui::MenuItem("Edit", nullptr, nullptr,
                            !m_project.userFunctions.empty())) {
          action = kEditCustomNodesActionId;
        }
        ImGui::EndMenu();
      }

      ImGui::Separator();

      ImGui::BeginDisabled(!m_project);
      if (ImGui::MenuItem("Compose", nullptr, nullptr, !m_livePreview)) {
        _composeMaterial();
      }
      if (ImGui::MenuItem("Compile", nullptr, nullptr, !m_livePreview)) {
        _forceUpdatePreview();
      }
      ImGui::Checkbox("Live preview", &m_livePreview);
      ImGui::EndDisabled();

      ImGui::EndMenu();
    }

    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical, 2.0f);

    ImGui::SetNextItemWidth(100);
    ImGui::BeginDisabled(!m_project);
    stageSelector("Stage", m_shaderType, gfx::isSurface(m_project.blueprint));
    ImGui::EndDisabled();

    ImGui::EndMenuBar();
  }

  if (action) ImGui::OpenPopup(*action, ImGuiWindowFlags_NoMove);

  constexpr auto kProjectExtension = ".mgproj";
  constexpr auto projectFilter = makeExtensionFilter(kProjectExtension);

  if (const auto p =
        showFileDialog(kSaveMaterialActionId,
                       {
                         .dir = currentDir,
                         .barrier = rootDir,
                         .entryFilter = projectFilter,
                         .forceExtension = kProjectExtension,
                         .flags = FileDialogFlags_AskOverwrite |
                                  FileDialogFlags_CreateDirectoryButton,
                       });
      p) {
    if (m_project.saveAs(*p)) {
      currentDir = p->parent_path();
    }
  }
  if (const auto p = showFileDialog(kLoadMaterialActionId,
                                    {
                                      .dir = currentDir,
                                      .barrier = rootDir,
                                      .entryFilter = projectFilter,
                                    });
      p) {
    if (_loadProject(*p)) {
      currentDir = p->parent_path();
    }
  }

  // ---

  const auto characterWidth = ImGui::CalcTextSize("#").x;
  const auto modalWidth = characterWidth * 80;

  if (auto data = m_customNodeWidget.showCreator(
        kCreateCustomNodeActionId, {modalWidth, 500}, m_project.userFunctions);
      data) {
    m_project.addUserFunction(std::move(*data));
  }

  auto changed = false;

  if (auto event = m_customNodeWidget.showEditor(
        kEditCustomNodesActionId, {modalWidth, 400}, m_project.userFunctions);
      event) {
    auto [customNodeAction, data] = *event;
    switch (customNodeAction) {
      using enum CustomNodeEditor::Action;

    case CodeChanged:
      changed |= m_project.isUsed(data);
      break;
    case Remove: {
      changed |= m_project.removeUserFunction(data).second;
    } break;
    }
  }

  return changed;
}
void MaterialEditor::_setupDockSpace(const ImGuiID dockspaceId) const {
  if (static auto firstTime = true; firstTime) [[unlikely]] {
    firstTime = false;

    if (ImGui::DockBuilderGetNode(dockspaceId) == nullptr) {
      ImGui::DockBuilderRemoveNode(dockspaceId);
      ImGui::DockBuilderAddNode(dockspaceId);

      auto centerNodeId = dockspaceId;

      auto leftNodeId = ImGui::DockBuilderSplitNode(
        centerNodeId, ImGuiDir_Left, 0.25f, nullptr, &centerNodeId);

      auto leftMidNodeId = ImGui::DockBuilderSplitNode(
        leftNodeId, ImGuiDir_Down, 0.65f, nullptr, &leftNodeId);
      auto leftBottomNodeId = ImGui::DockBuilderSplitNode(
        leftMidNodeId, ImGuiDir_Down, 0.65f, nullptr, &leftMidNodeId);

      const auto bottomNodeId = ImGui::DockBuilderSplitNode(
        centerNodeId, ImGuiDir_Down, 0.25f, nullptr, &centerNodeId);

      ImGui::DockBuilderDockWindow(GUI::Windows::kPreview, leftNodeId);
      ImGui::DockBuilderDockWindows(
        {
          GUI::Windows::kNodeList,
          GUI::Windows::kSettings,
        },
        leftMidNodeId);

      ImGui::DockBuilderDockWindows(
        {
          GUI::Windows::kRenderSettings,
          GUI::Windows::kSceneSettings,
        },
        leftBottomNodeId);

      ImGui::DockBuilderDockWindow(GUI::Windows::kLogger, bottomNodeId);
      ImGui::DockBuilderDockWindows(
        {
          GUI::Windows::kCanvas,
          GUI::Windows::kCodeEditor,
        },
        centerNodeId);

      ImGui::DockBuilderFinish(dockspaceId);
    }
  }
}

void MaterialEditor::_nodeListWidget(ShaderGraph &g) {
  for (const auto vd : g.vertices()) {
    if (const auto &prop = g.getVertexProp(vd);
        !bool(prop.flags & NodeFlags::Internal)) {
      const auto id = prop.id;
      if (auto selected = ImNodes::IsNodeSelected(id);
          ImGui::Selectable(prop.toString().c_str(), &selected)) {
        if (selected) {
          if (!ImGui::IsKeyDown(ImGuiMod_Ctrl)) {
            ImNodes::ClearNodeSelection();
          }
          ImNodes::SelectNode(id);
        } else {
          ImNodes::ClearNodeSelection(id);
        }
        ImNodes::EditorContextMoveToNodeCenter(id);
      }
    }
  }
}

bool MaterialEditor::_canvasWidget(
  ShaderGraph &g, const std::set<VertexDescriptor> &connectedVertices) {
  assert(hasRoot(g));

  ImNodes::BeginNodeEditor();

  constexpr auto kAddNodePopupId = IM_UNIQUE_ID;
  if (ImNodes::IsEditorHovered() &&
      ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
    ImGui::OpenPopup(kAddNodePopupId);
  }
  _nodePopup(kAddNodePopupId, g);
  auto changed = _inspectNodes(g, connectedVertices);
  _renderLinks(g);
  if (m_miniMap.enabled) ImNodes::MiniMap(m_miniMap.size, m_miniMap.location);

  ImNodes::EndNodeEditor();

  if (ImGui::IsWindowFocused(ImGuiHoveredFlags_ChildWindows)) {
    changed |= _handleNewLinks(g, connectedVertices);
    changed |= _handleDeletedLinks(g, connectedVertices);
    changed |= _handleDeletedNodes(g, connectedVertices);
  }
  return changed;
}

void MaterialEditor::_nodePopup(const char *name, ShaderGraph &g) {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {8, 8});
  if (ImGui::BeginPopup(name, ImGuiWindowFlags_NoMove)) {
    const auto clickPos = ImGui::GetMousePosOnOpeningCurrentPopup();

    static std::string pattern;
    ImGui::InputTextWithHint(IM_UNIQUE_ID, ICON_FA_FILTER " Filter", &pattern);
    ImGui::SameLine();
    if (ImGui::SmallButton(ICON_FA_ERASER)) pattern.clear();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    const auto entries =
      buildNodeMenuEntries(m_scriptedFunctions, m_project.userFunctions);
    if (const auto node = processNodeMenu(g, entries, pattern, m_shaderType,
                                          m_project.blueprint);
        node) {
      ImNodes::ClearSelection();
      const auto id = node->id;
      ImNodes::SetNodeScreenSpacePos(id, clickPos);
      ImNodes::SelectNode(id);
    }

    ImGui::EndPopup();
  }
  ImGui::PopStyleVar();
}
bool MaterialEditor::_inspectNodes(
  ShaderGraph &g, const std::set<VertexDescriptor> &connectedVertices) {
  auto changed = false;

  static std::optional<Request> request;
  if (request) {
    changed |= handleRequest(g, request, connectedVertices);
  }
  for (const auto vd : g.vertices()) {
    auto &vertexProp = g.getVertexProp(vd);
    assert(vertexProp.id >= 0);
    if (bool(vertexProp.flags & NodeFlags::Internal)) continue;

    ImGui::PushID(vertexProp.id);
    auto dirty = nodeWidget(g, vertexProp, m_project.blueprint);
    constexpr auto kContextMenuId = IM_UNIQUE_ID;
    ImGui::OpenPopupOnItemClick(kContextMenuId,
                                ImGuiPopupFlags_MouseButtonRight);
    if (!request) {
      const auto labelChanged = nodeContextMenu(kContextMenuId, g, vd, request);
      if (labelChanged &&
          holds_any_of<PropertyValue, TextureParam>(vertexProp.variant)) {
        dirty = true;
      }
    }
    ImGui::PopID();

    if (dirty && connectedVertices.contains(vd)) changed = true;
  }
  return changed;
}
void MaterialEditor::_renderLinks(const ShaderGraph &g) {
  for (const auto &&ed : g.edges()) {
    if (const auto &prop = g.getEdgeProp(ed); prop.type == EdgeType::Explicit) {
      const auto [from, to] = g.getConnection(ed);
      ImNodes::Link(prop.id, g.getVertexProp(from).id, g.getVertexProp(to).id);
    }
  }
}
bool MaterialEditor::_handleNewLinks(
  ShaderGraph &g, const std::set<VertexDescriptor> &connectedVertices) {
  if (std::pair<int32_t, int32_t> pair;
      ImNodes::IsLinkCreated(&pair.first, &pair.second)) {
    auto sourceVd = g.findVertex(pair.first);
    auto targetVd = g.findVertex(pair.second);
    assert(sourceVd && targetVd);

    constexpr auto isLinkValid = [](auto a, auto b) {
      return a != NodeFlags::None && b != NodeFlags::None && a != b;
    };

    const auto sourceFlags = g.getVertexProp(*sourceVd).flags;
    if (isLinkValid(sourceFlags, g.getVertexProp(*targetVd).flags)) {
      // Ensure the edge is always directed from the value to
      // whatever produces the value.
      if (sourceFlags != NodeFlags::Input) std::swap(sourceVd, targetVd);

      g.removeOutEdges(*sourceVd);
      const auto ed =
        g.addEdge(EdgeType::Explicit, {.from = *sourceVd, .to = *targetVd});
      if (isAcyclic(g)) {
        return connectedVertices.contains(*sourceVd);
      } else {
        g.removeEdge(ed);
        m_logger->warn(
          "Link has been discarded. Cycles in a shader graph are forbidden.");
      }
    }
  }
  return false;
}
bool MaterialEditor::_handleDeletedLinks(
  ShaderGraph &g, const std::set<VertexDescriptor> &connectedVertices) {
  auto changed = false;

  const auto removeLink = [&g, &connectedVertices, &changed](int32_t id) {
    changed |= ::removeLink(g, *g.findEdge(id), connectedVertices);
  };

  // Triggered when a link is clicked (if any of the following is set):
  // ImNodesIO::LinkDetachWithModifierClick
  // ImNodesAttributeFlags_EnableLinkDetachWithDragClick
  // (PushAttributeFlag)
  if (int32_t linkId; ImNodes::IsLinkDestroyed(&linkId)) {
    removeLink(linkId);
  }

  constexpr auto kConfirmDestructionId = MAKE_WARNING("Destroy Links");

  static std::vector<int32_t> selectedLinks;
  if (const auto numSelectedLinks = ImNodes::NumSelectedLinks();
      numSelectedLinks > 0 && ImGui::IsKeyReleased(ImGuiKey_Delete)) {
    selectedLinks.resize(numSelectedLinks);
    ImNodes::GetSelectedLinks(selectedLinks.data());

    ImGui::OpenPopup(kConfirmDestructionId);
  }

  if (const auto button =
        showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
          kConfirmDestructionId, "Remove selected links?");
      button && *button == ModalButton::Yes) {
    std::ranges::for_each(selectedLinks | std::views::filter(validId),
                          removeLink);
  }
  return changed;
}
bool MaterialEditor::_handleDeletedNodes(
  ShaderGraph &g, const std::set<VertexDescriptor> &connectedVertices) {
  auto changed = false;

  constexpr auto kConfirmDestructionId = MAKE_WARNING("Destroy Nodes");

  static std::vector<int32_t> selectedNodes;
  if (const auto numSelectedNodes = ImNodes::NumSelectedNodes();
      numSelectedNodes > 0 && ImGui::IsKeyReleased(ImGuiKey_Delete)) {
    selectedNodes.resize(numSelectedNodes);
    ImNodes::GetSelectedNodes(selectedNodes.data());

    // Ignore the master node (if selected).
    std::erase(selectedNodes, ShaderGraph::kRootNodeId);

    if (!selectedNodes.empty()) ImGui::OpenPopup(kConfirmDestructionId);
  }
  if (const auto button =
        showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
          kConfirmDestructionId, "Remove selected nodes?");
      button && *button == ModalButton::Yes) {
    for (const auto id : selectedNodes | std::views::filter(validId)) {
      changed |= removeNode(g, *g.findVertex(id), connectedVertices);
    }
  }
  return changed;
}

bool MaterialEditor::_settingsWidget() {
  ImGui::SetNextItemWidth(150);
  ImGui::InputText("Name", &m_project.name);

  ImGui::Dummy({0, 2});
  ImGui::PushItemWidth(100);
  const auto changed = inspect(m_project.blueprint);
  ImGui::PopItemWidth();

  return changed;
}
