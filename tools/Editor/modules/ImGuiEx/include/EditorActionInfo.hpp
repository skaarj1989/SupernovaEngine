#pragma once

#include <vector>
#include <span>

struct EditorActionInfo {
  const char *description;
  std::vector<const char *> shortcuts;
};

void keyboardShortcutsTable(const char *label,
                            std::span<const EditorActionInfo>);
