#include "WorldRendererWidget.hpp"
#include "IconsFontAwesome6.h"
#include "imgui.h"

namespace {

[[nodiscard]] auto toString(gfx::PipelineGroups groups) {
  switch (groups) {
    using enum gfx::PipelineGroups;

  case SurfaceMaterial:
    return "Surface";
  case PostProcessMaterial:
    return "PostProcess";
  case BuiltIn:
    return "BuiltIn";
  }
  assert(false);
  return "";
}

} // namespace

void showWorldRendererWindow(const char *name, bool *open,
                             gfx::WorldRenderer &renderer) {
  ZoneScopedN("WorldRendererWindow");
  if (ImGui::Begin(name, open)) {
    if (ImGui::CollapsingHeader("Pipelines", ImGuiTreeNodeFlags_DefaultOpen)) {
      constexpr auto kTableFlags =
        ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH |
        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoHostExtendX;
      if (ImGui::BeginTable(IM_UNIQUE_ID, 3, kTableFlags)) {
        ImGui::TableSetupColumn("Group");
        ImGui::TableSetupColumn("Count");
        ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_NoHeaderLabel);

        ImGui::TableHeadersRow();

        using enum gfx::PipelineGroups;
        for (const auto group :
             {SurfaceMaterial, PostProcessMaterial, BuiltIn}) {
          ImGui::TableNextRow();

          ImGui::TableSetColumnIndex(0);
          ImGui::Text(toString(group));

          ImGui::TableSetColumnIndex(1);
          const auto cacheSize = renderer.countPipelines(group);
          ImGui::Text("%u", cacheSize);

          ImGui::TableSetColumnIndex(2);
          ImGui::PushID(std::to_underlying(group));
          ImGui::BeginDisabled(cacheSize == 0);
          if (ImGui::SmallButton(ICON_FA_ERASER)) {
            renderer.getRenderDevice().waitIdle();
            renderer.clearPipelines(group);
          }
          ImGui::EndDisabled();
          ImGui::PopID();
        }
        ImGui::EndTable();
      }
    }
  }
  ImGui::End();
}
