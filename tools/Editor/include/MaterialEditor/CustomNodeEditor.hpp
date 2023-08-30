#pragma once

#include "UserFunctionData.hpp"
#include "TextEditor.h"

class CustomNodeCreator {
public:
  explicit CustomNodeCreator(TextEditor &);

  [[nodiscard]] std::optional<UserFunctionData>
  show(const char *name, ImVec2 size, const UserFunctions &);

private:
  void _reset();

private:
  TextEditor &m_textEditor;

  UserFunctionData m_data;
  bool m_valid{false};
};

class CustomNodeEditor {
public:
  explicit CustomNodeEditor(TextEditor &);

  enum class Action { CodeChanged, Remove };
  using Event = std::pair<Action, const UserFunctionData *>;

  [[nodiscard]] std::optional<Event> show(const char *name, ImVec2 size,
                                          UserFunctions &);

private:
  void _setData(UserFunctionData *data);
  void _reset();

private:
  TextEditor &m_textEditor;

  UserFunctionData *m_data{nullptr};
  bool m_dirty{false};
};

class CustomNodeWidget {
public:
  CustomNodeWidget() {
    m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
  }

  [[nodiscard]] auto showCreator(const char *name, ImVec2 size,
                                 const UserFunctions &userFunctions) {
    return m_creator.show(name, size, userFunctions);
  }
  [[nodiscard]] auto showEditor(const char *name, ImVec2 size,
                                UserFunctions &userFunctions) {
    return m_editor.show(name, size, userFunctions);
  }

private:
  TextEditor m_textEditor;
  CustomNodeCreator m_creator{m_textEditor};
  CustomNodeEditor m_editor{m_textEditor};
};
