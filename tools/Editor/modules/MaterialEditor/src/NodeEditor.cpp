#include "NodeEditor.hpp"
#include "ShaderGraph.hpp"

#include "MaterialProject.hpp"
#include "GraphCommands.hpp"
#include "ProjectCommands.hpp"

#include "FileDialog.hpp"
#include "ImGuiPopups.hpp"
#include "ImGuiModal.hpp"
#include "ImGuiDragAndDrop.hpp"

#include "ImNodesStyleWidget.hpp"
#include "ImNodesStyleJSON.hpp"
#include "ImNodesHelper.hpp"

#include "IconsFontAwesome6.h"
#include "ImGuiTitleBarMacro.hpp"
#include "imgui_internal.h" // MenuEx, Push/PopItemFlag
#include "imgui_stdlib.h"   // InputText(WithHint)

#include "tracy/Tracy.hpp"

#include <array>
#include <format>
#include <ranges>

namespace {

struct GUI {
  GUI() = delete;

  struct Actions {
    Actions() = delete;

    static constexpr auto kLoadStyle = MAKE_TITLE_BAR(ICON_FA_UPLOAD, "Load");
    static constexpr auto kSaveStyle =
      MAKE_TITLE_BAR(ICON_FA_FLOPPY_DISK, "Save As ...");
    static constexpr auto kEditStyle =
      MAKE_TITLE_BAR(ICON_FA_PAINTBRUSH, "Style");

    static constexpr auto kExportGraphviz =
      MAKE_TITLE_BAR(ICON_FA_FLOPPY_DISK, "Graphviz");

    static constexpr auto kCreateCustomNode =
      MAKE_TITLE_BAR(ICON_FA_PEN_TO_SQUARE, "Custom Node");
    static constexpr auto kEditCustomNodes =
      MAKE_TITLE_BAR(ICON_FA_PENCIL, "Custom Node");
  };
};

[[nodiscard]] auto findMenuEntry(MenuEntries &entries,
                                 const std::string_view label) {
  return std::find_if(entries.begin(), entries.end(),
                      [label](const MenuEntryVariant &variant) {
                        const auto *e = std::get_if<MenuEntry>(&variant);
                        return e && e->label == label;
                      });
}
bool purge(MenuEntries &entries, const std::string_view label) {
  if (const auto it = findMenuEntry(entries, label); it != entries.cend()) {
    std::get<MenuEntry>(*it).payload = MenuEntries{};
    return true;
  }
  return false;
}
bool refresh(MenuEntries &entries, const auto &functions,
             const std::string_view label) {
  if (const auto it = findMenuEntry(entries, label); it != entries.cend()) {
    std::get<MenuEntry>(*it).payload = buildNodeMenuEntries(functions);
    return true;
  }
  return false;
}

} // namespace

//
// NodeEditor class:
//

NodeEditor::NodeEditor(const NodeFactoryRegistry &registry)
    : m_nodeFactoryRegistry{&registry} {
  ImNodes::CreateContext();
  load("./assets/MaterialEditor/ImNodes.json", ImNodes::GetStyle());

#define SET_MODIFIER(Name, Key)                                                \
  ImNodes::GetIO().Name.Modifier = &ImGui::GetIO().Key

  SET_MODIFIER(LinkDetachWithModifierClick, KeyAlt);
  SET_MODIFIER(MultipleSelectModifier, KeyCtrl);
#undef SET_MODIFIER

  m_nodeMenuEntries = buildCoreMenuEntries();
}
NodeEditor::~NodeEditor() { ImNodes::DestroyContext(); }

void NodeEditor::showCanvas(MaterialProject &project,
                            const ShaderGraphStageViewMutable view,
                            spdlog::logger &logger) {
  ZoneScopedN("NodeEditor");
  m_currentProject = &project;

  m_focused = ImGui::IsWindowFocused(ImGuiHoveredFlags_ChildWindows);

  std::optional<const char *> action;
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Custom node", *m_currentProject)) {
      if (ImGui::MenuItem("Define", "Ctrl+M"))
        action = GUI::Actions::kCreateCustomNode;
      if (ImGui::MenuItem("Edit", "Ctrl+Alt+M", false,
                          m_currentProject->hasUserFunctions())) {
        action = GUI::Actions::kEditCustomNodes;
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
      if (ImGui::MenuItem("Select all", "Ctrl+A")) {
        _selectAllNodes(view);
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Undo", "Ctrl+Z", false,
                          m_currentProject->canUndo())) {
        m_currentProject->undo(Command::Context{logger});
      }
      if (ImGui::MenuItem("Redo", "Ctrl+Y", false,
                          m_currentProject->canRedo())) {
        m_currentProject->redo(Command::Context{logger});
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Copy", "Ctrl+C", ImNodes::NumSelectedNodes() > 0)) {
      }
      if (ImGui::MenuItem("Paste", "Ctrl+V")) {
      }
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Debug")) {
      if (ImGui::BeginMenuEx("Graphviz", ICON_FA_DIAGRAM_PROJECT)) {
        if (ImGui::MenuItemEx("File", ICON_FA_FLOPPY_DISK, "Ctrl+Alt+D")) {
          action = GUI::Actions::kExportGraphviz;
        }
        if (ImGui::MenuItemEx("Clipboard", ICON_FA_CLIPBOARD, "Ctrl+D")) {
          m_currentProject->addCommand<ShaderGraphToGraphvizCommand>(
            *view.graph);
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
      if (ImGui::MenuItem("Load")) action = GUI::Actions::kLoadStyle;
      if (ImGui::MenuItem("Save")) action = GUI::Actions::kSaveStyle;
      ImGui::Separator();
      if (ImGui::MenuItem("Edit")) action = GUI::Actions::kEditStyle;

      ImGui::EndMenu();
    }

    ImGui::Separator();

    ImGui::CheckboxFlags("Grid Snapping", &ImNodes::GetStyle().Flags,
                         ImNodesStyleFlags_GridSnapping);

    ImGui::EndMenuBar();
  }

  const auto &surface = m_currentProject->getBlueprint().surface;
  _nodeCanvas(view, surface ? &(*surface) : nullptr);

  if (m_focused && !action) {
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_A)) {
      _selectAllNodes(view);
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z)) {
      m_currentProject->undo(Command::Context{logger});
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Y)) {
      m_currentProject->redo(Command::Context{logger});
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_D)) {
      m_currentProject->addCommand<ShaderGraphToGraphvizCommand>(*view.graph);
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_M)) {
      action = GUI::Actions::kCreateCustomNode;
    } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Alt |
                                        ImGuiKey_M)) {
      if (m_currentProject->hasUserFunctions())
        action = GUI::Actions::kEditCustomNodes;
    }
  }

  if (action) ImGui::OpenPopup(*action, ImGuiWindowFlags_NoMove);

  constexpr auto kStyleExtension = ".json";
  static const auto styleFilter = makeExtensionFilter(kStyleExtension);

  static auto rootPath = std::filesystem::current_path();

  if (const auto p = showFileDialog(GUI::Actions::kLoadStyle,
                                    {
                                      .dir = rootPath,
                                      .entryFilter = styleFilter,
                                    });
      p) {
    load(*p, ImNodes::GetStyle());
  }
  if (const auto p =
        showFileDialog(GUI::Actions::kSaveStyle,
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

  if (const auto p =
        showFileDialog(GUI::Actions::kExportGraphviz,
                       {
                         .dir = rootPath,
                         .forceExtension = ".dot",
                         .flags = FileDialogFlags_AskOverwrite |
                                  FileDialogFlags_CreateDirectoryButton,
                       });
      p) {
    m_currentProject->addCommand<ShaderGraphToGraphvizCommand>(*view.graph, *p);
  }

  const auto characterWidth = ImGui::CalcTextSize("#").x;
  const auto modalWidth = characterWidth * 80;

  ImGui::SetNextWindowSize({modalWidth, 600}, ImGuiCond_FirstUseEver);
  if (ImGui::BeginPopupModal(GUI::Actions::kEditStyle)) {
    ImNodes::ShowStyleEditor();
    ImGui::Separator();
    if (ImGui::Button(ICON_FA_BAN " Close")) {
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  m_customNodeWidget.showCreator(GUI::Actions::kCreateCustomNode,
                                 {modalWidth, 500}, *m_currentProject);
  m_customNodeWidget.showEditor(GUI::Actions::kEditCustomNodes,
                                {modalWidth, 400}, *m_currentProject);
}
void NodeEditor::showNodeList(const ShaderGraphStageViewConst view) {
  ZoneScopedN("NodeEditor::NodeList");

  ImNodes::EditorContextSet(view.nodeEditorContext);
  for (const auto *node :
       view.graph->nodes() | std::views::filter(nonInternalNode)) {
    const auto id = node->vertex.id;
    ImGui::PushID(id);
    const auto label =
      std::format("[{}: {}] {}", id, node->toString(), node->label);
    if (auto selected = ImNodes::IsNodeSelected(id);
        ImGui::Selectable(label.c_str(), &selected)) {
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
    ImGui::PopID();
  }
}

constexpr auto kScriptedFunctionEntriesMenuName = "Scripted";
constexpr auto kUserFunctionEntriesMenuName = "User";

void NodeEditor::refreshFunctionEntries(
  const ScriptedFunctions &userFunctions) {
  refresh(m_nodeMenuEntries, userFunctions, kScriptedFunctionEntriesMenuName);
}
void NodeEditor::refreshFunctionEntries(const UserFunctions &userFunctions) {
  refresh(m_nodeMenuEntries, userFunctions, kUserFunctionEntriesMenuName);
}
void NodeEditor::purgeUserFunctionMenu() {
  purge(m_nodeMenuEntries, kUserFunctionEntriesMenuName);
}

//
// (private):
//

void NodeEditor::_displayNodeRegistryMenu(
  const char *name, const ShaderGraphStageViewMutable view,
  const gfx::Material::Surface *surface) {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {8, 8});
  if (ImGui::BeginPopup(name, ImGuiWindowFlags_NoMove)) {
    ZoneScopedN("NodeEditor::RegistryMenu");

    const auto clickPos = ImGui::GetMousePosOnOpeningCurrentPopup();

    static std::string pattern;
    ImGui::InputTextWithHint(IM_UNIQUE_ID, ICON_FA_FILTER " Filter", &pattern);
    ImGui::SameLine();
    if (ImGui::SmallButton(ICON_FA_ERASER)) pattern.clear();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (auto id = processNodeMenu(m_nodeMenuEntries, surface,
                                  view.graph->getShaderType(), pattern);
        id) {
      m_currentProject->addCommand<CreateNodeCommand>(
        view, *m_nodeFactoryRegistry, *id, clickPos);
    }

    ImGui::EndPopup();
  }
  ImGui::PopStyleVar();
}
void NodeEditor::_nodeCanvas(const ShaderGraphStageViewMutable view,
                             const gfx::Material::Surface *surface) {
  ZoneScopedN("NodeEditor::Canvas");

  ImNodes::EditorContextSet(view.nodeEditorContext);
  ImNodes::BeginNodeEditor();

  constexpr auto kAddNodePopupId = IM_UNIQUE_ID;
  if (ImNodes::IsEditorHovered() &&
      ImGui::IsMouseReleased(ImGuiMouseButton_Right)) {
    ImGui::OpenPopup(kAddNodePopupId);
  }
  _displayNodeRegistryMenu(kAddNodePopupId, view, surface);
  _inspectNodes(view, surface);
  _renderLinks(*view.graph);

  if (m_miniMap.enabled) ImNodes::MiniMap(m_miniMap.size, m_miniMap.location);
  ImNodes::EndNodeEditor();

  if (m_focused) {
    _handleNewLinks(*view.graph);
    _handleDeletion(view);
  }
}
void NodeEditor::_inspectNodes(const ShaderGraphStageViewMutable view,
                               const gfx::Material::Surface *surface) {
  ZoneScopedN("NodeEditor::InspectNodes");

  m_nodeUIVisitor.setContext(*view.graph, surface);
  for (auto *node : view.graph->nodes() | std::views::filter(nonInternalNode)) {
    ImGui::PushID(node->vertex.id);

    ImNodes::BeginNode(node->vertex);
    node->accept(m_nodeUIVisitor);
    ImNodes::EndNode();

    constexpr auto kContextMenuId = IM_UNIQUE_ID;
    ImGui::OpenPopupOnItemClick(kContextMenuId,
                                ImGuiPopupFlags_MouseButtonRight);
    _nodeContextMenu(kContextMenuId, view, node);

    ImGui::PopID();
  }
}
void NodeEditor::_nodeContextMenu(const char *name,
                                  const ShaderGraphStageViewMutable view,
                                  const NodeBase *node) {
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {8, 8});
  if (ImGui::BeginPopup(name, ImGuiWindowFlags_NoMove)) {
    ZoneScopedN("NodeEditor::NodeContextMenu");

    const auto id = node->vertex.id;
    const auto isMasterNode = id == ShaderGraph::kRootNodeId;
    ImGui::Text(ICON_FA_HASHTAG " Node ID: %d", id);
    if (!isMasterNode) {
      auto label = node->label;
      ImGui::SetNextItemWidth(100);
      const auto dirty = ImGui::InputTextWithHint(
        IM_UNIQUE_ID, "Label", &label, ImGuiInputTextFlags_EnterReturnsTrue);
      if (dirty) {
        m_currentProject->addCommand<RenameNodeCommand>(*node->graph, id,
                                                        label);
      }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::MenuItemEx("Clone", ICON_FA_CLONE, nullptr, false,
                          !isMasterNode)) {
      m_currentProject->addCommand<CloneNodeCommand>(view, id,
                                                     ImGui::GetMousePos());
    }

    ImGui::PushItemFlag(ImGuiItemFlags_AutoClosePopups, false);
    ImGui::MenuItemEx("Remove", ICON_FA_TRASH, nullptr, false, !isMasterNode);
    ImGui::PopItemFlag();

    attachPopup(nullptr, ImGuiMouseButton_Left, [this, view, id] {
      ImGui::TextUnformatted("Do you really want to remove the selected node?");
      ImGui::Separator();
      if (ImGui::Button(ICON_FA_TRASH " Yes")) {
        std::unordered_set<EdgeDescriptor> edges;
        collectExternalEdges(*view.graph, id, edges);
        for (const auto &ed : edges) {
          m_currentProject->addCommand<RemoveLinkCommand>(
            *view.graph, view.graph->getProperty(ed).id);
        }
        m_currentProject->addCommand<RemoveNodeCommand>(view, id);
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button(ICON_FA_BAN " No")) ImGui::CloseCurrentPopup();
    });

    ImGui::EndPopup();
  }
  ImGui::PopStyleVar();
}

void NodeEditor::_handleNewLinks(ShaderGraph &g) {
  if (ConnectedIDs ids; ImNodes::IsLinkCreated(&ids.first, &ids.second)) {
    using enum NodeBase::Flags;
    static constexpr auto isLinkValid = [](const auto a, const auto b) {
      return a != None && b != None && a != b;
    };
    const auto [source, target] = extractNodes(g, ids);
    if (isLinkValid(source->flags, target->flags)) {
      // Ensure the edge is always directed from the value to
      // whatever produces the value.
      if (source->flags != Input) {
        std::swap(ids.first, ids.second);
      }
      m_currentProject->addCommand<CreateLinkCommand>(g, ids);
    }
  }
}
void NodeEditor::_handleDeletion(const ShaderGraphStageViewMutable view) {
  // Triggered when a link is clicked (if any of the following is set):
  // ImNodesIO::LinkDetachWithModifierClick
  // ImNodesAttributeFlags_EnableLinkDetachWithDragClick
  // (PushAttributeFlag)
  if (EdgeID id; ImNodes::IsLinkDestroyed(&id)) {
    m_currentProject->addCommand<RemoveLinkCommand>(*view.graph, id);
  }

  constexpr auto kConfirmDestructionId = MAKE_WARNING("Destroy");

  static std::string message;
  static std::vector<EdgeID> selectedLinks;
  static std::vector<VertexID> selectedNodes;

  if (ImGui::IsKeyReleased(ImGuiKey_Delete)) {
    selectedLinks.clear();
    selectedNodes.clear();

    if (const auto count = ImNodes::NumSelectedLinks(); count > 0) {
      selectedLinks.resize(count);
      ImNodes::GetSelectedLinks(selectedLinks.data());
    }
    if (const auto count = ImNodes::NumSelectedNodes(); count > 0) {
      selectedNodes.resize(count);
      ImNodes::GetSelectedNodes(selectedNodes.data());
      // Ignore the master node (if selected).
      std::erase(selectedNodes, ShaderGraph::kRootNodeId);
    }
    if (!selectedLinks.empty() || !selectedNodes.empty()) {
      std::unordered_set<EdgeDescriptor> edges;
      for (const auto id : selectedNodes) {
        collectExternalEdges(*view.graph, id, edges);
      }
      for (const auto &ed : edges) {
        const auto id = view.graph->getProperty(ed).id;
        if (std::ranges::find(selectedLinks, id) == selectedLinks.cend()) {
          selectedLinks.push_back(id);
        }
      }
      message = std::format(
        "Confirm destruction of the following:\n-links: {}\n-nodes: {}",
        selectedLinks.size(), selectedNodes.size());

      ImGui::OpenPopup(kConfirmDestructionId);
    }
  }

  if (const auto button =
        showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
          kConfirmDestructionId, message.c_str());
      button == ModalButton::Yes) {
    static constexpr auto validId = [](const int32_t id) { return id >= 0; };
    for (const auto id : selectedLinks | std::views::filter(validId)) {
      m_currentProject->addCommand<RemoveLinkCommand>(*view.graph, id);
    }
    for (const auto id : selectedNodes | std::views::filter(validId)) {
      m_currentProject->addCommand<RemoveNodeCommand>(view, id);
    }
  }
}

void NodeEditor::_renderLinks(const ShaderGraph &g) {
  ZoneScopedN("NodeEditor::RenderLinks");
  for (const auto &[prop, c] : g.links() | std::views::filter(externalLink))
    ImNodes::Link(prop.id, c.source.id, c.target.id);
}

void NodeEditor::_selectAllNodes(const ShaderGraphStageViewMutable view) {
  ImNodes::EditorContextSet(view.nodeEditorContext);
  for (const auto *node :
       view.graph->nodes() | std::views::filter(nonInternalNode)) {
    if (!ImNodes::IsNodeSelected(node->vertex.id)) {
      ImNodes::SelectNode(node->vertex.id);
    }
  }
}
