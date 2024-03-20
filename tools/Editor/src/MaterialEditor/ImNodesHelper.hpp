#pragma once

#include "imnodes.h"
#include <optional>
#include <string>

namespace ImNodes {

void ClearSelections();

void EditorContextMoveToNodeCenter(const int32_t id);

struct InputAttributeParams {
  std::string_view name;
  std::optional<ImU32> color{};
  std::optional<ImNodesPinShape> pinShape{};
};
template <typename Func = std::monostate>
void AddInputAttribute(const int32_t id, const InputAttributeParams &params,
                       Func callback = {}) {
  assert(id >= 0);

  const auto active = params.pinShape.has_value();
  active ? ImNodes::BeginInputAttribute(id, *params.pinShape)
         : ImNodes::BeginStaticAttribute(id);

  ImGui::BeginDisabled(!active);
  if (!params.name.empty()) {
    if (params.color) ImGui::PushStyleColor(ImGuiCol_Text, *params.color);
    ImGui::TextUnformatted(params.name.data());
    if (params.color) ImGui::PopStyleColor();
  }
  if constexpr (std::is_invocable_v<Func>) callback();
  ImGui::EndDisabled();

  active ? ImNodes::EndInputAttribute() : ImNodes::EndStaticAttribute();
}

template <typename Func>
void AddOutputAttribute(const int32_t id,
                        const std::optional<ImNodesPinShape> pinShape,
                        Func callback) {
  assert(id >= 0);

  const auto active = pinShape.has_value();
  active ? BeginOutputAttribute(id, *pinShape) : BeginStaticAttribute(id);
  callback();
  active ? EndOutputAttribute() : EndStaticAttribute();
}

struct OutputAttributeParams {
  std::string_view name;
  std::optional<ImU32> color{};
  std::optional<ImNodesPinShape> pinShape{};
  float nodeWidth{0.0f};
};
void AddOutputAttribute(const int32_t id, const OutputAttributeParams &);

} // namespace ImNodes
