#pragma once

#include "EditorActionInfo.hpp"

class TextEditor;

void basicTextEditorWidget(TextEditor &, const char *name,
                           const bool border = false);
void addBasicEditMenu(TextEditor *);

[[nodiscard]] std::span<const EditorActionInfo> getBasicTextEditorActions();
