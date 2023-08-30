#pragma once

#include "imnodes.h"

#include <optional>
#include <string>

namespace ImNodes {

void ClearSelection();

void MiniMapWidget(ImNodesEditorContext *);

void EditorContextMoveToNodeCenter(int32_t id);

struct InputAttributeParams {
  std::string_view name;
  std::optional<ImU32> color{};
  ImNodesPinShape pinShape{ImNodesPinShape_CircleFilled};
  bool active{true};
};
template <typename Func = std::monostate>
void AddInputAttribute(int32_t id, const InputAttributeParams &params,
                       Func callback = {}) {
  assert(id >= 0);

  params.active ? ImNodes::BeginInputAttribute(id)
                : ImNodes::BeginStaticAttribute(id);

  const auto labelWidth = ImGui::CalcTextSize(params.name.data()).x;
  if (params.color) ImGui::PushStyleColor(ImGuiCol_Text, *params.color);
  ImGui::TextUnformatted(params.name.data());
  if (params.color) ImGui::PopStyleColor();
  if constexpr (std::is_invocable_v<Func, float>) callback(labelWidth);

  params.active ? ImNodes::EndInputAttribute() : ImNodes::EndStaticAttribute();
}

template <typename Func>
void AddOutputAttribute(int32_t id, bool active, Func callback) {
  assert(id >= 0);

  active ? BeginOutputAttribute(id) : BeginStaticAttribute(id);
  callback();
  active ? EndOutputAttribute() : EndStaticAttribute();
}

struct OutputAttributeParams {
  std::string_view name;
  std::optional<ImU32> color{};
  ImNodesPinShape pinShape{ImNodesPinShape_CircleFilled};
  bool active{true};
  float nodeWidth{0.0f};
};
void AddOutputAttribute(int32_t id, const OutputAttributeParams &);

} // namespace ImNodes
