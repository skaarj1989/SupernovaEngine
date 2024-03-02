#include "TextEditorCommon.hpp"
#include "ImGuiTitleBarMacro.hpp"
#include "IconsFontAwesome6.h"
#include "TextEditor.h"
#include "ImGuiModal.hpp"
#include "imgui_internal.h" // ClosePopupsExceptModals

namespace {

const std::vector<TextEditorAction> kDefaults{
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
                           bool border) {
  constexpr auto kShowKeyboardShortcuts_ActionId =
    MAKE_TITLE_BAR(ICON_FA_CIRCLE_QUESTION, "Help");

  if (ImGui::BeginChild(name, {0, 0},
                        border ? ImGuiChildFlags_Border : ImGuiChildFlags_None,
                        ImGuiWindowFlags_MenuBar)) {
    auto hasFocus = ImGui::IsWindowFocused();

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
      keyboardShortcutsTable("Basic editing", getDefaultActions());
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

[[nodiscard]] std::span<const TextEditorAction> getDefaultActions() {
  return kDefaults;
}

void keyboardShortcutsTable(const char *label,
                            std::span<const TextEditorAction> entries) {
  ImGui::SeparatorText(label);
  if (ImGui::BeginTable(IM_UNIQUE_ID, 2,
                        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
                          ImGuiTableFlags_SizingFixedFit)) {
    for (const auto &[description, shortcuts] : entries) {
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      for (const auto *chord : shortcuts) {
        ImGui::Text(chord);
      }
      ImGui::TableSetColumnIndex(1);
      ImGui::Text(description);
    }
    ImGui::EndTable();
  }
}
