#pragma once

#include "UserFunctionData.hpp"
#include "TextEditor.h"

class TextEditorController {
protected:
  explicit TextEditorController(TextEditor &);

  [[nodiscard]] bool _isChanged() const;

  void _load(const std::string &);
  virtual void _reset();

protected:
  TextEditor &m_textEditor;
  int32_t m_undoIndex{0};
};

class CustomNodeCreator : TextEditorController {
public:
  explicit CustomNodeCreator(TextEditor &);

  [[nodiscard]] std::optional<UserFunctionData>
  show(const char *name, ImVec2 size, const UserFunctions &);

private:
  void _reset() override;

private:
  bool m_valid{false};
  UserFunctionData m_data;
};

class CustomNodeEditor : TextEditorController {
public:
  explicit CustomNodeEditor(TextEditor &);

  enum class Action { CodeChanged, Remove };
  using Event = std::pair<Action, const UserFunctionData *>;

  [[nodiscard]] std::optional<Event> show(const char *name, ImVec2 size,
                                          UserFunctions &);

private:
  void _setData(UserFunctionData *data);
  void _reset() override;

private:
  UserFunctionData *m_data{nullptr};
};

class CustomNodeWidget {
public:
  CustomNodeWidget();

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
