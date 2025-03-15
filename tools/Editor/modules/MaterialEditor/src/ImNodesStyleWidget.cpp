#include "ImNodesStyleWidget.hpp"
#include "imnodes.h"

namespace ImNodes {

namespace {

[[nodiscard]] auto GetStyleColorName(const ImNodesCol idx) {
#define CASE(Value)                                                            \
  case ImNodesCol_##Value:                                                     \
    return #Value

  switch (idx) {
    CASE(NodeBackground);
    CASE(NodeBackgroundHovered);
    CASE(NodeBackgroundSelected);
    CASE(NodeOutline);
    CASE(TitleBar);
    CASE(TitleBarHovered);
    CASE(TitleBarSelected);
    CASE(Link);
    CASE(LinkHovered);
    CASE(LinkSelected);
    CASE(Pin);
    CASE(PinHovered);
    CASE(BoxSelector);
    CASE(BoxSelectorOutline);
    CASE(GridBackground);
    CASE(GridLine);
    CASE(GridLinePrimary);
    CASE(MiniMapBackground);
    CASE(MiniMapBackgroundHovered);
    CASE(MiniMapOutline);
    CASE(MiniMapOutlineHovered);
    CASE(MiniMapNodeBackground);
    CASE(MiniMapNodeBackgroundHovered);
    CASE(MiniMapNodeBackgroundSelected);
    CASE(MiniMapNodeOutline);
    CASE(MiniMapLink);
    CASE(MiniMapLinkSelected);
    CASE(MiniMapCanvas);
    CASE(MiniMapCanvasOutline);
  }
#undef CASE

  assert(false);
  return "Undefined";
}

} // namespace

// Based on ImGui::ShowStyleEditor(ImGuiStyle *) from imgui_demo.cpp
void ShowStyleEditor(ImNodesStyle *ref) {
  auto &style = GetStyle();
  static ImNodesStyle refSavedStyle;

  // Default to using internal storage as reference
  if (static bool init = true; init && ref == nullptr) {
    refSavedStyle = style;
    init = false;
  }
  if (ref == nullptr) ref = &refSavedStyle;

  ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

  // Save/Revert button
  if (ImGui::Button("Save Ref")) *ref = refSavedStyle = style;
  ImGui::SameLine();
  if (ImGui::Button("Revert Ref")) style = *ref;

  ImGui::Separator();

  if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_None)) {
    if (ImGui::BeginTabItem("Sizes")) {
      ImGui::SeparatorText("Grid");
      ImGui::SliderFloat("GridSpacing", &style.GridSpacing, 0.0f, 24.0f * 10,
                         "%.1f");

      ImGui::SeparatorText("Nodes");
      ImGui::SliderFloat("NodeCornerRounding", &style.NodeCornerRounding, 0.0f,
                         4.0f * 6, "%.1f");
      ImGui::SliderFloat2("NodePadding", &style.NodePadding.x, 0.0f, 8.0f * 2,
                          "%.1f");
      ImGui::SliderFloat("NodeBorderThickness", &style.NodeBorderThickness,
                         0.0f, 1.0f * 10, "%.1f");

      ImGui::SeparatorText("Links");
      ImGui::SliderFloat("LinkThickness", &style.LinkThickness, 0.0f, 3.0f * 4,
                         "%.1f");
      ImGui::SliderFloat("LinkLineSegmentsPerLength",
                         &style.LinkLineSegmentsPerLength, 0.0f, 0.1f * 10,
                         "%.2f");
      ImGui::SliderFloat("LinkHoverDistance", &style.LinkHoverDistance, 1.0f,
                         10.0f * 2, "%.1f");

      ImGui::SeparatorText("Pins");
      ImGui::SliderFloat("PinCircleRadius", &style.PinCircleRadius, 1.0f,
                         4.0f * 2, "%.1f");
      ImGui::SliderFloat("PinQuadSideLength", &style.PinQuadSideLength, 1.0f,
                         7.0f, "%.1f");
      ImGui::SliderFloat("PinTriangleSideLength", &style.PinTriangleSideLength,
                         1.0f, 9.5f, "%.1f");
      ImGui::SliderFloat("PinLineThickness", &style.PinLineThickness, 0.1f,
                         1.0f * 4, "%.1f");
      ImGui::SliderFloat("PinHoverRadius", &style.PinHoverRadius, 0.1f,
                         10.0f * 2, "%.1f");
      ImGui::SliderFloat("PinOffset", &style.PinOffset, 0.0f, 0.0f + 5.0f,
                         "%.1f");

      ImGui::SeparatorText("MiniMap");
      ImGui::SliderFloat2("MiniMapPadding", &style.MiniMapPadding.x, 0.0f,
                          8.0f * 2, "%.1f");
      ImGui::SliderFloat2("MiniMapOffset", &style.MiniMapOffset.x, 0.0f,
                          4.0f * 2, "%.1f");

      ImGui::SeparatorText("Misc");
      if (ImGui::TreeNodeEx("Flags")) {
        ImGui::CheckboxFlags("ImNodesStyleFlags_NodeOutline", &style.Flags,
                             ImNodesStyleFlags_NodeOutline);
        ImGui::CheckboxFlags("ImNodesStyleFlags_GridLines", &style.Flags,
                             ImNodesStyleFlags_GridLines);
        ImGui::CheckboxFlags("ImNodesStyleFlags_GridLinesPrimary", &style.Flags,
                             ImNodesStyleFlags_GridLinesPrimary);
        ImGui::CheckboxFlags("ImNodesStyleFlags_GridSnapping", &style.Flags,
                             ImNodesStyleFlags_GridSnapping);
        ImGui::TreePop();
      }

      ImGui::EndTabItem();
    }
    if (ImGui::BeginTabItem("Colors")) {
      static ImGuiTextFilter filter;
      filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

      static ImGuiColorEditFlags alphaFlags{ImGuiColorEditFlags_None};
      if (ImGui::RadioButton("Opaque",
                             alphaFlags == ImGuiColorEditFlags_AlphaOpaque)) {
        alphaFlags = ImGuiColorEditFlags_AlphaOpaque;
      }
      ImGui::SameLine();
      if (ImGui::RadioButton("Alpha", alphaFlags == ImGuiColorEditFlags_None)) {
        alphaFlags = ImGuiColorEditFlags_None;
      }
      ImGui::SameLine();
      if (ImGui::RadioButton("Both", alphaFlags ==
                                       ImGuiColorEditFlags_AlphaPreviewHalf)) {
        alphaFlags = ImGuiColorEditFlags_AlphaPreviewHalf;
      }

      const auto kItemInnerSpacingX = ImGui::GetStyle().ItemInnerSpacing.x;

      ImGui::SetNextWindowSizeConstraints(
        {0.0f, ImGui::GetTextLineHeightWithSpacing() * 10}, {FLT_MAX, FLT_MAX});
      ImGui::BeginChild("##colors", {0, 0}, ImGuiChildFlags_Borders,
                        ImGuiWindowFlags_AlwaysVerticalScrollbar |
                          ImGuiWindowFlags_AlwaysHorizontalScrollbar |
                          ImGuiChildFlags_NavFlattened);
      ImGui::PushItemWidth(ImGui::GetFontSize() * -16);
      for (auto i = 0; i < ImNodesCol_COUNT; ++i) {
        const auto *name = GetStyleColorName(i);
        if (!filter.PassFilter(name)) continue;

        ImGui::PushID(i);
        auto color = ImGui::ColorConvertU32ToFloat4(style.Colors[i]);
        if (ImGui::ColorEdit4("##color", &color.x,
                              ImGuiColorEditFlags_AlphaBar | alphaFlags)) {
          style.Colors[i] = ImGui::ColorConvertFloat4ToU32(color);
        }
        if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(uint32_t)) != 0) {
          ImGui::SameLine(0.0f, kItemInnerSpacingX);
          if (ImGui::Button("Save")) {
            ref->Colors[i] = style.Colors[i];
          }
          ImGui::SameLine(0.0f, kItemInnerSpacingX);
          if (ImGui::Button("Revert")) {
            style.Colors[i] = ref->Colors[i];
          }
        }
        ImGui::SameLine(0.0f, kItemInnerSpacingX);
        ImGui::TextUnformatted(name);
        ImGui::PopID();
      }
      ImGui::PopItemWidth();
      ImGui::EndChild();

      ImGui::EndTabItem();
    }

    ImGui::EndTabBar();
  }

  ImGui::PopItemWidth();
}

} // namespace ImNodes
