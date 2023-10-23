#include "ScriptEditor.hpp"
#include "Services.hpp"

#include "ImGuiTitleBarMacro.hpp"
#include "ImGuiModal.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "FileDialog.hpp"
#include "imgui_internal.h"

#include <fstream>

namespace {

[[nodiscard]] auto makeLabel(const std::filesystem::path &p) {
  assert(!p.empty());
  return p.filename().string();
}
[[nodiscard]] auto makeTooltip(const std::filesystem::path &p) {
  assert(!p.empty());
  return os::FileSystem::relativeToRoot(p)->generic_string();
}

} // namespace

//
// ScriptEditor::Entry struct:
//

ScriptEditor::Entry::Entry(std::shared_ptr<ScriptResource> r)
    : resource{std::move(r)} {
  textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
  if (resource) textEditor.SetText(resource->code);
}

void ScriptEditor::Entry::save() {
  assert(hasResource());
  saveAs(resource->getPath());
}
void ScriptEditor::Entry::saveAs(const std::filesystem::path &p) {
  if (auto code = textEditor.GetText(); hasResource()) {
    resource->code = std::move(code);
  } else {
    resource = std::make_shared<ScriptResource>(std::move(code), p);
  }
  if (std::ofstream f{p, std::ios::binary}; f.is_open()) {
    f << resource->code;
    dirty = false;
  }
}

bool ScriptEditor::Entry::hasResource() const { return resource && *resource; }

//
// ScriptEditor class:
//

void ScriptEditor::openScript(std::shared_ptr<ScriptResource> r) {
  if (!hasScript(r)) m_scripts.emplace_back(std::move(r));
}

bool ScriptEditor::hasScript(const std::shared_ptr<ScriptResource> &r) const {
  return std::ranges::find_if(m_scripts, [&r](const Entry &e) {
           return e.resource == r;
         }) != m_scripts.cend();
}

void ScriptEditor::show(const char *name, bool *popen) {
  if (ImGui::Begin(name, popen, ImGuiWindowFlags_MenuBar)) {
    // Feature: Drag'n'Drop a file/ script resource to open it in a new tab.
    const auto cursorPos = ImGui::GetCursorPos();
    ImGui::Dummy(ImGui::GetContentRegionAvail());
    if (ImGui::BeginDragDropTarget()) {
      if (auto incomingResource = extractResourceFromPayload<ScriptManager>(
            kImGuiPayloadTypeScript, Services::Resources::Scripts::value());
          incomingResource) {
        openScript(incomingResource->handle());
      }
      ImGui::EndDragDropTarget();
    }
    ImGui::SetCursorPos(cursorPos);

    constexpr auto kSaveScriptAs_ActionId =
      MAKE_TITLE_BAR(ICON_FA_FILE, "Save As...");
    constexpr auto kOpenScript_ActionId = MAKE_TITLE_BAR(ICON_FA_FILE, "Open");

    static std::optional<std::size_t> junk;

    std::optional<const char *> action;
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItemEx("New ...", ICON_FA_FILE)) {
          m_scripts.emplace_back();
        }
        auto *entry = _getActiveEntry();
        const auto canSave = entry && entry->dirty;
        if (ImGui::MenuItemEx("Save", ICON_FA_DOWNLOAD, nullptr, false,
                              canSave)) {
          if (entry->hasResource()) {
            entry->save();
          } else {
            action = kSaveScriptAs_ActionId;
          }
        }
        if (ImGui::MenuItemEx("Open", ICON_FA_UPLOAD)) {
          action = kOpenScript_ActionId;
        }

        ImGui::Separator();

        if (ImGui::MenuItemEx("Close", ICON_FA_XMARK, nullptr, false,
                              m_activeScriptId.has_value())) {
          junk = *m_activeScriptId;
        }

        ImGui::EndMenu();
      }

      if (ImGui::MenuItem(ICON_FA_PLAY " Execute", nullptr, nullptr,
                          m_activeScriptId.has_value())) {
        publish(RunScriptRequest{_getActiveEntry()->textEditor.GetText()});
      }

      ImGui::EndMenuBar();
    }

    if (action) ImGui::OpenPopup(*action);

    constexpr auto kScriptExtension = ".lua";
    constexpr auto entryFilter = makeExtensionFilter(kScriptExtension);

    const auto &rootDir = os::FileSystem::getRoot();
    static auto currentDir = rootDir;
    if (const auto p =
          showFileDialog(kSaveScriptAs_ActionId,
                         {
                           .dir = currentDir,
                           .barrier = rootDir,
                           .entryFilter = entryFilter,
                           .forceExtension = kScriptExtension,
                           .flags = FileDialogFlags_AskOverwrite |
                                    FileDialogFlags_CreateDirectoryButton,
                         });
        p) {
      _getActiveEntry()->saveAs(*p);
    }
    if (const auto p = showFileDialog(kOpenScript_ActionId,
                                      {
                                        .dir = currentDir,
                                        .barrier = rootDir,
                                        .entryFilter = entryFilter,
                                      });
        p) {
      openScript(loadResource<ScriptManager>(p->string()));
    }

    // ---

    ImGui::BeginTabBar(IM_UNIQUE_ID, ImGuiTabBarFlags_AutoSelectNewTabs |
                                       ImGuiTabBarFlags_Reorderable);
    for (auto i = 0u; i < m_scripts.size(); ++i) {
      auto &[r, textEditor, dirty] = m_scripts[i];
      if (textEditor.IsTextChanged()) {
        dirty = !r || r->code.compare(textEditor.GetText()) != 0;
      }

      auto open = true;
      const auto label =
        std::format("{}##{}", r ? makeLabel(r->getPath()) : "(empty)", i);
      const auto visible = ImGui::BeginTabItem(
        label.c_str(), &open,
        dirty ? ImGuiTabItemFlags_UnsavedDocument : ImGuiTabItemFlags_None);

      if (ImGui::IsKeyDown(ImGuiMod_Alt) && ImGui::IsItemHovered()) {
        ImGui::ShowTooltip(r ? makeTooltip(r->getPath()) : "(unsaved)");
      }

      if (visible) {
        m_activeScriptId = i;
        textEditor.Render(IM_UNIQUE_ID);
        ImGui::EndTabItem();
      }

      if (!open) junk = i;
    }
    ImGui::EndTabBar();

    constexpr auto kDiscardScriptActionId = MAKE_WARNING("Discard?");

    if (junk) {
      if (m_scripts[*junk].dirty)
        ImGui::OpenPopup(kDiscardScriptActionId);
      else {
        _removeScriptAt(*junk);
        junk = std::nullopt;
      }
    }

    if (const auto button =
          showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
            kDiscardScriptActionId, "Do you really want to discard changes?");
        button) {
      if (*button == ModalButton::Yes) _removeScriptAt(*junk);
      junk = std::nullopt;
    }
  }
  ImGui::End();
}

//
// (private):
//

ScriptEditor::Entry *ScriptEditor::_getActiveEntry() {
  return m_activeScriptId ? &m_scripts[*m_activeScriptId] : nullptr;
}

void ScriptEditor::_removeScriptAt(std::size_t index) {
  assert(index >= 0 && index < m_scripts.size());
  m_scripts.erase(m_scripts.begin() + index);

  if (m_activeScriptId && *m_activeScriptId == index)
    m_activeScriptId = std::nullopt;
}
