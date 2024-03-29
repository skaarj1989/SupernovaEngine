#pragma once

#include "UserFunction.hpp"
#pragma warning(push, 0)
#include "TextEditor.h"
#pragma warning(pop)

class MaterialProject;

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

  void show(const char *name, const ImVec2 size, MaterialProject &);

private:
  void _reset() override;

private:
  bool m_valid{false};
  UserFunction::Data m_data;
};

class CustomNodeEditor : TextEditorController {
public:
  explicit CustomNodeEditor(TextEditor &);

  void show(const char *name, const ImVec2 size, MaterialProject &);

private:
  void _setData(const UserFunction::ID, UserFunction::Data *);
  void _reset() override;

private:
  std::optional<UserFunction::ID> m_functionId{0};
};

class CustomNodeWidget {
public:
  CustomNodeWidget();

  void showCreator(const char *name, const ImVec2 size,
                   MaterialProject &project) {
    m_creator.show(name, size, project);
  }
  void showEditor(const char *name, const ImVec2 size,
                  MaterialProject &project) {
    m_editor.show(name, size, project);
  }

private:
  TextEditor m_textEditor;
  CustomNodeCreator m_creator{m_textEditor};
  CustomNodeEditor m_editor{m_textEditor};
};
