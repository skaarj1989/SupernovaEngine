#include "ScriptEditor.hpp"

#include "ImGuiTitleBarMacro.hpp"
#include "ImGuiModal.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "FileDialog.hpp"
#include "TextEditorCommon.hpp"

#include "tracy/Tracy.hpp"

#include <format>
#include <ranges>

namespace {

constexpr auto kScriptExtension = ".lua";
const auto scriptFileFilter = makeExtensionFilter(kScriptExtension);

struct Action {
  Action() = delete;

  static constexpr auto kOpenScript = MAKE_TITLE_BAR(ICON_FA_FILE, "Open");
  static constexpr auto kSaveScriptAs =
    MAKE_TITLE_BAR(ICON_FA_FILE, "Save As...");
  static constexpr auto kDiscardScript = MAKE_WARNING("Discard?");
  static constexpr auto kShowKeyboardShortcuts =
    MAKE_TITLE_BAR(ICON_FA_QUESTION, "Help");
};

[[nodiscard]] auto makeLabel(const std::filesystem::path &p) {
  return p.empty() ? "(empty)" : p.filename().string();
}
[[nodiscard]] auto makeTooltip(const std::filesystem::path &p) {
  return p.empty() ? "(empty)"
                   : os::FileSystem::relativeToRoot(p)->generic_string();
}

void showKeyboardShortcuts() {
  static const std::vector<EditorActionInfo> kScriptEditorActions{
    {"New Script", {"Ctrl+N"}},
    {"Open Script", {"Ctrl+O"}},
    {"Save", {"Ctrl+S"}},
    {"Save As", {"Ctrl+Shift+S"}},
    {"Close Script", {"Ctrl+W"}},

    {"Go to next script (tab)", {"Ctrl+PgUp"}},
    {"Go to previous script (tab)", {"Ctrl+PgDn"}},
  };
  keyboardShortcutsTable("Scripts", kScriptEditorActions);
  keyboardShortcutsTable("Basic editing", getBasicTextEditorActions());
}

} // namespace

//
// ScriptEditor class:
//

void ScriptEditor::newScript() { m_scripts.emplace_back(); }

bool ScriptEditor::open(const std::filesystem::path &p) {
  if (os::FileSystem::getExtension(p) == kScriptExtension && !contains(p)) {
    m_scripts.emplace_back(p);
    return true;
  }
  return false;
}
bool ScriptEditor::contains(const std::filesystem::path &p) const {
  return std::ranges::find_if(m_scripts, [&p](const Entry &e) {
           return e.path == p;
         }) != m_scripts.cend();
}

void ScriptEditor::show(const char *name, bool *popen) {
  if (ImGui::Begin(name, popen, ImGuiWindowFlags_MenuBar)) {
    ZoneScopedN("ScriptEditor");
    auto hasFocus = ImGui::IsWindowFocused();

    // Feature: Drag'n'Drop a file to open it in a new tab.
    const auto cursorPos = ImGui::GetCursorPos();
    ImGui::Dummy(ImGui::GetContentRegionAvail());
    if (ImGui::BeginDragDropTarget()) {
      if (auto *payload = ImGui::AcceptDragDropPayload(kImGuiPayloadTypeFile);
          payload) {
        if (open(ImGui::ExtractPath(*payload))) ImGui::SetWindowFocus();
      }
      ImGui::EndDragDropTarget();
    }
    ImGui::SetCursorPos(cursorPos);

    std::optional<const char *> action;
    const auto save = [&action](Entry *entry) {
      assert(entry != nullptr);
      if (entry->isOnDisk()) {
        entry->save();
      } else {
        action = Action::kSaveScriptAs;
      }
    };

    if (ImGui::BeginMenuBar()) {
      auto *entry = _getActiveEntry();
      if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New", "Ctrl+N")) {
          newScript();
        }
        if (ImGui::MenuItem("Open Script...", "Ctrl+O")) {
          action = Action::kOpenScript;
        }
        const auto canSave = entry && entry->isChanged();
        if (ImGui::MenuItem("Save", "Ctrl+S", nullptr, canSave)) {
          save(entry);
        }
        if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S", nullptr, canSave)) {
          action = Action::kSaveScriptAs;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Close", "Ctrl+W", nullptr, entry)) {
          m_junk = *m_activeScriptId;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Exit")) {
          if (popen) *popen = false;
        }
        ImGui::EndMenu();
      }
      addBasicEditMenu(entry ? &entry->textEditor : nullptr);

      if (ImGui::BeginMenu("Run")) {
        if (ImGui::MenuItem("Execute", "F5", nullptr, entry)) {
          _runScript(*entry);
        }
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("Keyboard shortcuts", "Ctrl+H")) {
          action = Action::kShowKeyboardShortcuts;
        }
        ImGui::EndMenu();
      }

      ImGui::EndMenuBar();
    }

    ImGui::BeginTabBar(IM_UNIQUE_ID, ImGuiTabBarFlags_AutoSelectNewTabs |
                                       ImGuiTabBarFlags_Reorderable);
    std::optional<std::size_t> forceActiveTab;
    if (size() > 1) {
      if (ImGui::GetIO().KeyCtrl) {
        if (ImGui::IsKeyPressed(ImGuiKey_PageUp)) {
          forceActiveTab = *m_activeScriptId + 1;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_PageDown)) {
          forceActiveTab = *m_activeScriptId - 1;
        }
      }
    }
    for (auto [i, entry] : std::views::enumerate(m_scripts)) {
      const auto label = std::format("{}##{}", makeLabel(entry.path), i);
      auto open = true;
      ImGuiTabItemFlags flags = entry.isChanged()
                                  ? ImGuiTabItemFlags_UnsavedDocument
                                  : ImGuiTabItemFlags_None;
      if (forceActiveTab == i) flags |= ImGuiTabItemFlags_SetSelected;
      const auto visible = ImGui::BeginTabItem(label.c_str(), &open, flags);
      if (ImGui::IsKeyDown(ImGuiMod_Alt) && ImGui::IsItemHovered()) {
        ImGui::ShowTooltip(makeTooltip(entry.path));
      }
      if (visible) {
        m_activeScriptId = i;
        hasFocus |= entry.textEditor.Render(IM_UNIQUE_ID, hasFocus);
        if (hasFocus) {
          if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift |
                                       ImGuiKey_S)) {
            action = Action::kSaveScriptAs;
          } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S)) {
            save(&entry);
          } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_R)) {
            entry.load();
          } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_W)) {
            m_junk = i;
          } else if (ImGui::IsKeyPressed(ImGuiKey_F5)) {
            _runScript(entry);
          }
        }
        ImGui::EndTabItem();
      }

      if (!open) m_junk = i;
    }
    ImGui::EndTabBar();

    if (hasFocus) {
      if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_N)) {
        newScript();
      } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_O)) {
        action = Action::kOpenScript;
      } else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_H)) {
        action = Action::kShowKeyboardShortcuts;
      }
    }

    if (m_junk) {
      if (m_scripts[*m_junk].isChanged()) {
        action = Action::kDiscardScript;
      } else {
        _removeScriptAt(*m_junk);
        m_junk = std::nullopt;
      }
    }

    // ---

    if (action) ImGui::OpenPopup(*action);

    if (const auto button =
          showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
            Action::kDiscardScript, "Do you really want to discard changes?");
        button) {
      if (*button == ModalButton::Yes) _removeScriptAt(*m_junk);
      m_junk = std::nullopt;
    }

    const auto &rootDir = os::FileSystem::getRoot();
    static auto currentDir = rootDir;
    if (const auto p = showFileDialog(Action::kOpenScript,
                                      {
                                        .dir = currentDir,
                                        .barrier = rootDir,
                                        .entryFilter = scriptFileFilter,
                                      });
        p) {
      open(*p);
    }
    if (const auto p =
          showFileDialog(Action::kSaveScriptAs,
                         {
                           .dir = currentDir,
                           .barrier = rootDir,
                           .entryFilter = scriptFileFilter,
                           .forceExtension = kScriptExtension,
                           .flags = FileDialogFlags_AskOverwrite |
                                    FileDialogFlags_CreateDirectoryButton,
                         });
        p) {
      _getActiveEntry()->saveAs(*p);
    }

    showModal<ModalButtons::Ok>(Action::kShowKeyboardShortcuts,
                                showKeyboardShortcuts);
  }
  ImGui::End();
}

//
// (private):
//

ScriptEditor::Entry *ScriptEditor::_getActiveEntry() {
  return m_activeScriptId ? &m_scripts[*m_activeScriptId] : nullptr;
}
void ScriptEditor::_runScript(const Entry &e) {
  publish(RunScriptRequest{e.textEditor.GetText()});
}
void ScriptEditor::_removeScriptAt(std::size_t index) {
  assert(index >= 0 && index < m_scripts.size());
  m_scripts.erase(m_scripts.begin() + index);

  if (m_activeScriptId && *m_activeScriptId == index)
    m_activeScriptId = std::nullopt;
}

//
// ScriptEditor::Entry struct:
//

ScriptEditor::Entry::Entry(const std::filesystem::path &p) : path{p} {
  textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Lua);
  textEditor.SetPalette(TextEditor::PaletteId::Dark);
  textEditor.SetShortTabsEnabled(true);
  textEditor.SetTabSize(2);

  load();
}

void ScriptEditor::Entry::load() {
  if (!path.empty()) {
    if (auto text = os::FileSystem::readText(path); text) {
      textEditor.SetText(*text);
    }
  }
}

void ScriptEditor::Entry::save() {
  assert(os::FileSystem::getExtension(path) == kScriptExtension);
  os::FileSystem::saveText(path, textEditor.GetText());
  undoIndex = textEditor.GetUndoIndex();
}
void ScriptEditor::Entry::saveAs(const std::filesystem::path &p) {
  if (p != path) {
    path = p;
    save();
  }
}
