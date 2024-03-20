#pragma once

#include "EditorActionInfo.hpp"

class TextEditor;

void basicTextEditorWidget(TextEditor &, const char *name, bool border = false);
void addBasicEditMenu(TextEditor *);

[[nodiscard]] std::span<const EditorActionInfo> getBasicTextEditorActions();
