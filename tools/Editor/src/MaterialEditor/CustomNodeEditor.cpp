#include "MaterialEditor/CustomNodeEditor.hpp"
#include "ImGuiTitleBarMacro.hpp"
#include "ImGuiModal.hpp"
#include "imgui_internal.h" // {Push/Pop}ItemFlag
#include "imgui_stdlib.h"   // InputText{WithHint}
#include <algorithm> // all_of
#include <span>

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
showParameterConfig(std::vector<UserFunctionData::Parameter> &inputs,
                    ImVec2 size) {
  auto dirty = false;
  std::optional<std::size_t> junk;

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
    for (auto &[dataType, name] : inputs) {
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

[[nodiscard]] auto showDependencyConfig(std::vector<std::size_t> &dependencies,
                                        const rhi::ShaderStages shaderStages,
                                        const UserFunctions &functions) {
  auto dirty = false;

  ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
  ImGui::SetNextItemWidth(150);
  if (ImGui::BeginCombo("Dependencies", nullptr, ImGuiComboFlags_NoPreview)) {
    for (const auto &[hash, data] : functions) {
      const auto it = std::ranges::find(dependencies, hash);
      const auto selected = it != dependencies.cend();
      const auto available =
        (data->shaderStages & shaderStages) == shaderStages;

      if (ImGui::MenuItem(buildDeclaration(*data).c_str(), nullptr, selected,
                          available)) {
        if (selected) {
          std::erase(dependencies, hash);
        } else {
          dependencies.push_back(hash);
        }
        dirty = true;
      }
    }
    ImGui::EndCombo();
  }
  ImGui::PopItemFlag();

  return dirty;
}

[[nodiscard]] bool isValid(const rhi::ShaderStages shaderStages) {
  return std::to_underlying(shaderStages) != 0;
}

[[nodiscard]] bool
isValid(std::span<const UserFunctionData::Parameter> inputs) {
  static const auto validParameter = [](const auto &p) {
    return p.dataType != DataType::Undefined && !p.name.empty();
  };
  return std::ranges::all_of(inputs, validParameter);
}
[[nodiscard]] bool isValid(const UserFunctionData &data) {
  return data.output != DataType::Undefined && isValid(data.shaderStages) &&
         !data.name.empty() && !data.code.empty() && isValid(data.inputs);
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
  }
  assert(false);
  return "-";
}

void sanitize(std::vector<std::size_t> &dependencies,
              const rhi::ShaderStages shaderStages,
              const UserFunctions &functions) {
  std::erase_if(dependencies, [shaderStages, &functions](std::size_t hash) {
    const auto it = functions.find(hash);
    return it != functions.cend()
             ? (it->second->shaderStages & shaderStages) != shaderStages
             : true;
  });
}

} // namespace

//
// CustomNodeCreator class:
//

CustomNodeCreator::CustomNodeCreator(TextEditor &textEditor)
    : m_textEditor{textEditor} {}

std::optional<UserFunctionData>
CustomNodeCreator::show(const char *name, ImVec2 size,
                        const UserFunctions &userFunctions) {
  std::optional<UserFunctionData> result;

  constexpr auto kModalFlags =
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
  if (ImGui::BeginPopupModal(name, nullptr, kModalFlags)) {
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

      ImGui::SeparatorText("Code");
      if (!userFunctions.empty()) {
        dirty |= showDependencyConfig(m_data.dependencies, m_data.shaderStages,
                                      userFunctions);
      }

      m_textEditor.SetReadOnly(false);
      m_textEditor.Render(IM_UNIQUE_ID);
      ImGui::EndChild();
    }

    if (m_textEditor.IsTextChanged()) {
      m_data.code = m_textEditor.GetText();
      dirty |= true;
    }

    if (dirty) m_valid = isValid(m_data);

    ImGui::BeginDisabled(!m_valid);
    if (ImGui::Button(ICON_FA_CHECK " Add")) {
      result = std::move(m_data);
      _reset();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndDisabled();

    constexpr auto kConfirmDiscardChanges = MAKE_WARNING("Confirm");

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_BAN " Close")) {
      ImGui::OpenPopup(kConfirmDiscardChanges);
    }

    if (auto button = showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
          kConfirmDiscardChanges, "Discard changes?");
        button && *button == ModalButton::Yes) {
      _reset();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }

  return result;
}

void CustomNodeCreator::_reset() {
  m_textEditor.SetText("");
  m_data = {};
  m_valid = false;
}

//
// CustomNodeEditor class:
//

CustomNodeEditor::CustomNodeEditor(TextEditor &textEditor)
    : m_textEditor{textEditor} {}

std::optional<CustomNodeEditor::Event>
CustomNodeEditor::show(const char *name, ImVec2 size,
                       UserFunctions &functions) {
  std::optional<CustomNodeEditor::Event> result;

  constexpr auto kConfirmDiscardChanges = MAKE_WARNING("Confirm");
  constexpr auto kConfirmDestruction = MAKE_WARNING("Confirm");

  constexpr auto kModalFlags =
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
  if (ImGui::BeginPopupModal(name, nullptr, kModalFlags)) {
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
          for (auto &[_, data] : functions) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text(toString(data->shaderStages));

            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(i);

            const auto label = buildDeclaration(*data.get());
            auto isSelected = data.get() == m_data;
            if (ImGui::Selectable(label.c_str(), &isSelected,
                                  ImGuiSelectableFlags_SpanAllColumns |
                                    ImGuiSelectableFlags_AllowOverlap |
                                    ImGuiSelectableFlags_DontClosePopups)) {
              if (m_dirty) {
                ImGui::OpenPopup(kConfirmDiscardChanges);
              } else {
                _setData(isSelected ? data.get() : nullptr);
              }
            }
            if (const auto button =
                  showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
                    kConfirmDiscardChanges, "Discard changes?");
                button && *button == ModalButton::Yes) {
              _setData(isSelected ? nullptr : data.get());
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
                button && *button == ModalButton::Yes) {
              result = Event{Action::Remove, data.get()};
              if (isSelected) m_data = nullptr;
            }
            ImGui::PopID();

            ++i;
          }
          ImGui::EndTable();
        }
      }

      ImGui::SeparatorText("Code");
      m_textEditor.SetReadOnly(m_data == nullptr);
      m_textEditor.Render(IM_UNIQUE_ID);

      ImGui::EndChild();
    }

    if (m_data && m_textEditor.IsTextChanged()) {
      m_dirty = m_textEditor.GetText() != m_data->code;
    }

    ImGui::BeginDisabled(m_dirty == false);
    if (ImGui::Button(ICON_FA_CHECK " Save")) {
      if (m_data) {
        m_data->code = m_textEditor.GetText();
        result = Event{Action::CodeChanged, m_data};
        m_dirty = false;
      }
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_BAN " Close")) {
      if (m_dirty) {
        ImGui::OpenPopup(kConfirmDiscardChanges);
      } else {
        _reset();
        ImGui::CloseCurrentPopup();
      }
    }
    if (const auto button =
          showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
            kConfirmDiscardChanges, "Discard changes?");
        button && *button == ModalButton::Yes) {
      _reset();
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
  }
  return result;
}

void CustomNodeEditor::_setData(UserFunctionData *data) {
  m_data = data;
  m_dirty = false;
  m_textEditor.SetText(m_data ? m_data->code : "");
}
void CustomNodeEditor::_reset() { _setData(nullptr); }
