#pragma once

#include <vector>
#include <span>

class TextEditor;

void basicTextEditorWidget(TextEditor &, const char *name, bool border = false);
void addBasicEditMenu(TextEditor *);

struct TextEditorAction {
  const char *description;
  std::vector<const char *> shortcuts;
};
[[nodiscard]] std::span<const TextEditorAction> getDefaultActions();

void keyboardShortcutsTable(const char *label,
                            std::span<const TextEditorAction>);
