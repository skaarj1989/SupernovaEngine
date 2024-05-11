#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImNodesHelper.hpp"
#include "imnodes_internal.h" // ImNodesEditorContext

namespace ImNodes {

void ClearSelections() {
  ClearLinkSelection();
  ClearNodeSelection();
}

void EditorContextMoveToNodeCenter(const int32_t id) {
  const auto canvasSize = GetCurrentContext()->CanvasRectScreenSpace.GetSize();
  const auto nodeSize = glm::vec2{GetNodeDimensions(id)};
  const auto offset = (canvasSize * 0.5f) - (nodeSize * 0.5f);
  const auto nodePos = -glm::vec2{GetNodeGridSpacePos(id)};
  ImNodes::EditorContextResetPanning(nodePos + offset);
}

void AddOutputAttribute(const int32_t id, const OutputAttributeParams &params) {
  AddOutputAttribute(id, params.pinShape, [&params] {
    const auto labelWidth = ImGui::CalcTextSize(params.name.data()).x;
    auto w = params.nodeWidth - labelWidth;
    if (w <= 0.0f) w = 0.0f;
    ImGui::Indent(w);
    if (params.color) ImGui::PushStyleColor(ImGuiCol_Text, *params.color);
    ImGui::TextUnformatted(params.name.data());
    if (params.color) ImGui::PopStyleColor();
    ImGui::Unindent(w);
  });
}

} // namespace ImNodes
