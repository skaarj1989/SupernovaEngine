#include "TextEditorCommon.hpp"
#include "os/FileSystem.hpp"

#include "ImGuiTitleBarMacro.hpp"
#include "IconsFontAwesome6.h"
#include "TextEditor.h"
#include "ImGuiModal.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "imgui_internal.h" // ClosePopupsExceptModals

namespace {

const std::vector<EditorActionInfo> kBasicActions{
  {"Select all", {"Ctrl+A"}},

  {"Go to beginning/end of line", {"Home/End"}},
  {"Go to beginning of file", {"Ctrl+Home"}},
  {"Go to end of file", {"Ctrl+End"}},

  {
    "Move line up/down",
    {"Ctrl+Shift+" ICON_FA_ARROW_UP "/" ICON_FA_ARROW_DOWN},
  },
  {"Delete line", {"Ctrl+Shift+K"}},

  {"Copy line/selection", {"Ctrl+C", "Ctrl+Insert"}},
  {"Paste", {"Ctrl+V", "Shift+Insert"}},
  {"Cut line/selection", {"Ctrl+X", "Shift+Del"}},

  {"Undo", {"Ctrl+Z", "Alt+Backspace"}},
  {"Redo", {"Ctrl+Y", "Ctrl+Shift+Z"}},
};

} // namespace

void basicTextEditorWidget(TextEditor &textEditor, const char *name,
                           const bool border) {
  constexpr auto kShowKeyboardShortcuts_ActionId =
    MAKE_TITLE_BAR(ICON_FA_CIRCLE_QUESTION, "Help");

  if (ImGui::BeginChild(name, {0, 0},
                        border ? ImGuiChildFlags_Border : ImGuiChildFlags_None,
                        ImGuiWindowFlags_MenuBar)) {
    auto hasFocus = ImGui::IsWindowFocused();

    const auto cursorPos = ImGui::GetCursorPos();
    ImGui::Dummy(ImGui::GetContentRegionAvail());
    if (ImGui::BeginDragDropTarget()) {
      if (auto *payload = ImGui::AcceptDragDropPayload(kImGuiPayloadTypeFile);
          payload) {
        if (const auto text =
              os::FileSystem::readText(ImGui::ExtractPath(*payload));
            text) {
          textEditor.SetText(text.value());
          ImGui::SetWindowFocus();
        }
      }
      ImGui::EndDragDropTarget();
    }
    ImGui::SetCursorPos(cursorPos);

    auto showKeyboardShortcuts = false;
    if (ImGui::BeginMenuBar()) {
      ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
      addBasicEditMenu(&textEditor);
      if (ImGui::BeginMenu("Help")) {
        if (ImGui::MenuItem("Keyboard shortcuts", "Ctrl+H")) {
          showKeyboardShortcuts = true;
          ImGui::ClosePopupsExceptModals();
        }
        ImGui::EndMenu();
      }
      ImGui::PopItemFlag();

      ImGui::EndMenuBar();
    }
    hasFocus |= textEditor.Render(IM_UNIQUE_ID, hasFocus);
    if (hasFocus && ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_H)) {
      showKeyboardShortcuts = true;
    }
    ImGui::EndChild();

    if (showKeyboardShortcuts)
      ImGui::OpenPopup(kShowKeyboardShortcuts_ActionId);

    showModal<ModalButtons::Ok>(kShowKeyboardShortcuts_ActionId, [] {
      keyboardShortcutsTable("Basic editing", getBasicTextEditorActions());
    });
  }
}
void addBasicEditMenu(TextEditor *textEditor) {
  if (ImGui::BeginMenu("Edit")) {
    if (ImGui::MenuItem("Undo", "Ctrl+Z", nullptr,
                        textEditor && textEditor->CanUndo())) {
      textEditor->Undo();
      ImGui::ClosePopupsExceptModals();
    }
    if (ImGui::MenuItem("Redo", "Ctrl+Y", nullptr,
                        textEditor && textEditor->CanRedo())) {
      textEditor->Redo();
      ImGui::ClosePopupsExceptModals();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Cut", "Ctrl+X", nullptr, textEditor)) {
      textEditor->Cut();
      ImGui::ClosePopupsExceptModals();
    }
    if (ImGui::MenuItem("Copy", "Ctrl+C", nullptr, textEditor)) {
      textEditor->Copy();
      ImGui::ClosePopupsExceptModals();
    }
    if (ImGui::MenuItem("Paste", "Ctrl+V", nullptr, textEditor)) {
      textEditor->Paste();
      ImGui::ClosePopupsExceptModals();
    }
    ImGui::EndMenu();
  }
}

[[nodiscard]] std::span<const EditorActionInfo> getBasicTextEditorActions() {
  return kBasicActions;
}
