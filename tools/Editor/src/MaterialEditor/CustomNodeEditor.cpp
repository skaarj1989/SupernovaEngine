#include "MaterialEditor/CustomNodeEditor.hpp"
#include "MaterialEditor/MaterialProject.hpp"

#include "ImGuiTitleBarMacro.hpp"
#include "ImGuiModal.hpp"
#include "TextEditorCommon.hpp"
#include "imgui_internal.h" // {Push/Pop}ItemFlag
#include "imgui_stdlib.h"   // InputText{WithHint}

#include "entt/signal/dispatcher.hpp"
#include "spdlog/logger.h"

#include "tracy/Tracy.hpp"

namespace {

[[nodiscard]] auto changeDataTypeCombo(const char *name, DataType &dataType) {
  auto dirty = false;

  using enum DataType;

  constexpr auto kNumOptions = 5 * 4 + 3 + 2;
  constexpr auto kNumSeparators = 6;
  // clang-format off
  constexpr auto kOptions =
    std::array<std::optional<DataType>, kNumOptions + kNumSeparators>{
      Bool, BVec2, BVec3, BVec4,
      std::nullopt,
      Int32, IVec2, IVec3, IVec4,
      std::nullopt,
      UInt32, UVec2, UVec3, UVec4,
      std::nullopt,
      Float, Vec2, Vec3, Vec4,
      std::nullopt,
      Double, DVec2, DVec3, DVec4,
      std::nullopt,
      Mat2, Mat3, Mat4,
      std::nullopt,
      Sampler2D, SamplerCube,
    };
  // clang-format on

  if (ImGui::BeginCombo(name, toString(dataType))) {
    for (const auto &option : kOptions) {
      if (option) {
        const auto value = *option;
        auto selected = value == dataType;
        if (ImGui::Selectable(toString(value), &selected,
                              selected ? ImGuiSelectableFlags_Disabled
                                       : ImGuiSelectableFlags_None)) {
          dataType = value;
          dirty = true;
        }
        if (selected) ImGui::SetItemDefaultFocus();
      } else {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
      }
    }
    ImGui::EndCombo();
  }
  return dirty;
}

[[nodiscard]] auto
showParameterConfig(std::vector<UserFunction::Data::Parameter> &inputs,
                    const ImVec2 size) {
  auto dirty = false;
  std::optional<UserFunction::ID> junk;

  constexpr auto kTableFlags =
    ImGuiTableFlags_BordersInner | ImGuiTableFlags_BordersOuter |
    ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollY;
  if (ImGui::BeginTable(IM_UNIQUE_ID, 3, kTableFlags, size)) {
    const auto width = ImGui::GetContentRegionAvail().x;
    ImGui::TableSetupColumn("DataType", ImGuiTableColumnFlags_WidthFixed,
                            width * 0.3f);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed,
                            width * 0.6f);
    ImGui::TableSetupColumn("Action",
                            ImGuiTableColumnFlags_NoHeaderLabel |
                              ImGuiTableColumnFlags_WidthFixed,
                            width * 0.1f);
    ImGui::TableHeadersRow();

    auto i = 0;
    for (auto &[name, dataType] : inputs) {
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      ImGui::PushID(i);
      ImGui::SetNextItemWidth(-FLT_MIN);
      dirty |= changeDataTypeCombo(IM_UNIQUE_ID, dataType);
      ImGui::PopID();

      ImGui::TableSetColumnIndex(1);
      ImGui::PushID(&name);
      ImGui::SetNextItemWidth(-FLT_MIN);
      dirty |= ImGui::InputText(IM_UNIQUE_ID, &name);
      ImGui::PopID();

      ImGui::TableSetColumnIndex(2);
      ImGui::PushID(i);
      if (ImGui::SmallButton(ICON_FA_TRASH)) {
        junk = i;
      }
      ImGui::PopID();

      ++i;
    }
    ImGui::EndTable();
  }

  if (junk) {
    inputs.erase(inputs.begin() + *junk);
    dirty = true;
  }

  if (ImGui::Button(ICON_FA_PLUS " Add parameter")) {
    inputs.emplace_back();
    dirty = true;
  }

  return dirty;
}

[[nodiscard]] auto
showDependencyConfig(std::vector<UserFunction::ID> &dependencies,
                     const rhi::ShaderStages shaderStages,
                     const UserFunctions &functions) {
  auto dirty = false;

  ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
  ImGui::SetNextItemWidth(150);
  if (ImGui::BeginCombo("Dependencies", nullptr, ImGuiComboFlags_NoPreview)) {
    for (const auto &[id, data] : functions) {
      const auto it = std::ranges::find(dependencies, id);
      const auto selected = it != dependencies.cend();
      const auto available =
        (data->shaderStages & shaderStages) == shaderStages;

      if (ImGui::MenuItem(buildDeclaration(*data).c_str(), nullptr, selected,
                          available)) {
        if (selected) {
          std::erase(dependencies, id);
        } else {
          dependencies.push_back(id);
        }
        dirty = true;
      }
    }
    ImGui::EndCombo();
  }
  ImGui::PopItemFlag();

  return dirty;
}

[[nodiscard]] auto toString(const rhi::ShaderStages stages) {
  switch (stages) {
    using enum rhi::ShaderStages;

  case Vertex:
    return "VS";
  case Fragment:
    return "FS";
  case Vertex | Fragment:
    return "VS|FS";

  default:
    assert(false);
    return "-";
  }
}

void sanitize(std::vector<UserFunction::ID> &dependencies,
              const rhi::ShaderStages shaderStages,
              const UserFunctions &functions) {
  std::erase_if(
    dependencies, [shaderStages, &functions](const UserFunction::ID id) {
      const auto it = functions.find(id);
      return it != functions.cend()
               ? (it->second->shaderStages & shaderStages) != shaderStages
               : true;
    });
}

class AddUserFunctionCommand : public UndoableCommand {
public:
  AddUserFunctionCommand(MaterialProject &project, UserFunction::Data &&data)
      : m_project{&project}, m_data{std::move(data)} {}

  bool execute(const Context &ctx) override {
    ctx.logger.trace("AddUserFunctionCommand::execute");
    m_id = m_project->addUserFunction(std::move(m_data));
    return m_id.has_value();
  }
  bool undo(const Context &ctx) override {
    ctx.logger.trace("AddUserFunctionCommand::undo");
    return m_project->removeUserFunction(*m_id);
  }

private:
  MaterialProject *m_project;

  UserFunction::Data m_data;
  std::optional<UserFunction::ID> m_id;
};
class UpdateUserFunctionCommand : public UndoableCommand {
public:
  UpdateUserFunctionCommand(MaterialProject &project, const UserFunction::ID id,
                            std::string &&code)
      : m_project{&project}, m_functionId{id}, m_code{std::move(code)} {}

  bool execute(const Context &ctx) override {
    ctx.logger.trace("UpdateUserFunctionCommand::execute");
    return _updateUserFunctionCode();
  }
  bool undo(const Context &ctx) {
    ctx.logger.trace("UpdateUserFunctionCommand::undo");
    return _updateUserFunctionCode();
  }

private:
  bool _updateUserFunctionCode() {
    if (auto previousCode =
          m_project->updateUserFunction(m_functionId, std::move(m_code));
        previousCode) {
      m_code = std::move(*previousCode);
      return true;
    }
    return false;
  }

private:
  MaterialProject *m_project;
  UserFunction::ID m_functionId;
  std::string m_code;
};
class RemoveUserFunctionCommand : public Command {
public:
  RemoveUserFunctionCommand(MaterialProject &project, const UserFunction::ID id)
      : m_project{&project}, m_functionId{id} {}

  bool execute(const Context &ctx) override {
    ctx.logger.trace("[RemoveUserFunctionCommand::execute]");

    const auto scope = m_project->getUserFunctionScope(m_functionId);
    for (const auto &[g, subGraph] : scope.stages) {
      for (const auto &ed : subGraph.edges) {
        g->remove(ed);
      }
      for (const auto vd : subGraph.vertices) {
        g->remove(vd);
      }
      ctx.logger.info("[stage={}] Removed: {} Links, {} Nodes",
                      rhi::toString(g->getShaderType()), subGraph.edges.size(),
                      subGraph.vertices.size());
    }
    for (auto id : scope.functionList) {
      m_project->removeUserFunction(id);
    }
    ctx.logger.info("Removed: {} User function(s)", scope.functionList.size());
    return true;
  }

private:
  MaterialProject *m_project;
  UserFunction::ID m_functionId;
};

} // namespace

//
// TextEditorController class:
//

TextEditorController::TextEditorController(TextEditor &textEditor)
    : m_textEditor{textEditor} {}

bool TextEditorController::_isChanged() const {
  return m_textEditor.GetUndoIndex() != m_undoIndex;
}

void TextEditorController::_load(const std::string &str) {
  m_textEditor.SetText(str);
  m_undoIndex = 0;
}
void TextEditorController::_reset() { _load(""); }

//
// CustomNodeCreator class:
//

CustomNodeCreator::CustomNodeCreator(TextEditor &textEditor)
    : TextEditorController{textEditor} {}

void CustomNodeCreator::show(const char *name, ImVec2 size,
                             MaterialProject &project) {
  constexpr auto kModalFlags =
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
  if (ImGui::BeginPopupModal(name, nullptr, kModalFlags)) {
    ZoneScopedN("CustomNodeCreator");

    const auto &userFunctions = project.getUserFunctions();

    auto dirty = false;
    if (ImGui::BeginChild(IM_UNIQUE_ID, size)) {
      ImGui::SeparatorText("Shader stages:");
      dirty |= ImGui::CheckboxFlags("Vertex", m_data.shaderStages,
                                    rhi::ShaderStages::Vertex);
      ImGui::SameLine();
      dirty |= ImGui::CheckboxFlags("Fragment", m_data.shaderStages,
                                    rhi::ShaderStages::Fragment);

      if (dirty) {
        sanitize(m_data.dependencies, m_data.shaderStages, userFunctions);
      }

      ImGui::SeparatorText("Signature");
      ImGui::SetNextItemWidth(100);
      dirty |= changeDataTypeCombo(IM_UNIQUE_ID, m_data.output);

      ImGui::SameLine();
      ImGui::SetNextItemWidth(-FLT_MIN);
      dirty |=
        ImGui::InputTextWithHint(IM_UNIQUE_ID, "Function name", &m_data.name);

      ImGui::SeparatorText("Parameters");
      dirty |= showParameterConfig(m_data.inputs, {0, 80});

      if (!userFunctions.empty()) {
        dirty |= showDependencyConfig(m_data.dependencies, m_data.shaderStages,
                                      userFunctions);
      }

      m_textEditor.SetReadOnlyEnabled(false);
      basicTextEditorWidget(m_textEditor, IM_UNIQUE_ID, true);

      ImGui::EndChild();
    }
    if (_isChanged()) {
      m_data.code = m_textEditor.GetText();
      m_undoIndex = m_textEditor.GetUndoIndex();
      dirty |= true;
    }
    if (dirty) m_valid = m_data.isValid();

    ImGui::BeginDisabled(!m_valid);
    if (ImGui::Button(ICON_FA_CHECK " Add")) {
      project.addCommand<AddUserFunctionCommand>(project, std::move(m_data));
      _reset();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();

    constexpr auto kConfirmDiscardChanges = MAKE_WARNING("Confirm");

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_BAN " Close")) {
      if (m_data != UserFunction::Data{}) {
        ImGui::OpenPopup(kConfirmDiscardChanges);
      } else {
        ImGui::CloseCurrentPopup();
      }
    }

    if (const auto button =
          showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
            kConfirmDiscardChanges, "Discard changes?");
        button == ModalButton::Yes) {
      _reset();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
}

void CustomNodeCreator::_reset() {
  TextEditorController::_reset();
  m_valid = false;
  m_data = {};
}

//
// CustomNodeEditor class:
//

CustomNodeEditor::CustomNodeEditor(TextEditor &textEditor)
    : TextEditorController{textEditor} {}

void CustomNodeEditor::show(const char *name, ImVec2 size,
                            MaterialProject &project) {
  constexpr auto kConfirmDiscardChanges = MAKE_WARNING("Confirm");
  constexpr auto kConfirmDestruction = MAKE_WARNING("Confirm");

  constexpr auto kModalFlags =
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
  if (ImGui::BeginPopupModal(name, nullptr, kModalFlags)) {
    ZoneScopedN("CustomNodeEditor");

    if (ImGui::BeginChild(IM_UNIQUE_ID, size)) {
      if (ImGui::CollapsingHeader("Functions",
                                  ImGuiTreeNodeFlags_DefaultOpen)) {
        constexpr auto kTableFlags =
          ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH |
          ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchProp |
          ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_ScrollY;
        if (ImGui::BeginTable(IM_UNIQUE_ID, 3, kTableFlags, {0, 100})) {
          ImGui::TableSetupColumn("Stage", ImGuiTableColumnFlags_WidthFixed,
                                  size.x * 0.1f);
          ImGui::TableSetupColumn("Signature", ImGuiTableColumnFlags_WidthFixed,
                                  size.x * 0.8f);
          ImGui::TableSetupColumn("Action",
                                  ImGuiTableColumnFlags_WidthFixed |
                                    ImGuiTableColumnFlags_NoHeaderLabel,
                                  size.x * 0.1f);
          ImGui::TableHeadersRow();

          auto i = 0;
          for (const auto &[id, data] : project.getUserFunctions()) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(toString(data->shaderStages));

            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(i);

            const auto label = buildDeclaration(*data.get());
            auto isSelected = id == m_functionId;
            if (ImGui::Selectable(label.c_str(), &isSelected,
                                  ImGuiSelectableFlags_SpanAllColumns |
                                    ImGuiSelectableFlags_AllowOverlap |
                                    ImGuiSelectableFlags_DontClosePopups)) {
              if (_isChanged()) {
                ImGui::OpenPopup(kConfirmDiscardChanges);
              } else {
                _setData(id, isSelected ? data.get() : nullptr);
              }
            }
            if (const auto button =
                  showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
                    kConfirmDiscardChanges, "Discard changes?");
                button == ModalButton::Yes) {
              _setData(id, isSelected ? nullptr : data.get());
            }

            ImGui::PopID();

            ImGui::TableSetColumnIndex(2);
            ImGui::PushID(i);
            if (ImGui::SmallButton(ICON_FA_TRASH)) {
              ImGui::OpenPopup(kConfirmDestruction);
            }
            if (const auto button =
                  showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
                    kConfirmDestruction,
                    "Do you really want to remove this function?\n"
                    "- All functions that depend on this one will also be "
                    "removed.\n"
                    "- All nodes that use this function (or depend on it) will "
                    "be destroyed.");
                button == ModalButton::Yes) {
              project.addCommand<RemoveUserFunctionCommand>(project, id);
              if (isSelected) m_functionId = std::nullopt;
            }
            ImGui::PopID();

            ++i;
          }
          ImGui::EndTable();
        }
      }

      m_textEditor.SetReadOnlyEnabled(!m_functionId);
      basicTextEditorWidget(m_textEditor, IM_UNIQUE_ID, true);

      ImGui::EndChild();
    }

    ImGui::BeginDisabled(!_isChanged());
    if (ImGui::Button(ICON_FA_CHECK " Save")) {
      assert(m_functionId);
      m_undoIndex = m_textEditor.GetUndoIndex();
      project.addCommand<UpdateUserFunctionCommand>(project, *m_functionId,
                                                    m_textEditor.GetText());
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_BAN " Close")) {
      if (_isChanged()) {
        ImGui::OpenPopup(kConfirmDiscardChanges);
      } else {
        _reset();
        ImGui::CloseCurrentPopup();
      }
    }
    if (const auto button =
          showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
            kConfirmDiscardChanges, "Discard changes?");
        button == ModalButton::Yes) {
      _reset();
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
}

void CustomNodeEditor::_setData(UserFunction::ID id, UserFunction::Data *data) {
  _load(data ? data->code : "");
  m_functionId = data ? std::make_optional(id) : std::nullopt;
}
void CustomNodeEditor::_reset() { _setData(0, nullptr); }

//
// CustomNodeWidget class:
//

CustomNodeWidget::CustomNodeWidget() {
  m_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinitionId::Glsl);
  m_textEditor.SetPalette(TextEditor::PaletteId::Dark);
  m_textEditor.SetShortTabsEnabled(true);
  m_textEditor.SetShowWhitespacesEnabled(false);
  m_textEditor.SetTabSize(2);
}
