#include "Inspectors/PropertyInspector.hpp"
#include "VisitorHelper.hpp"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp" // value_ptr

bool inspect(std::vector<gfx::Property> &properties) {
  auto dirty = false;

  constexpr auto kTableFlags = ImGuiTableFlags_Borders |
                               ImGuiTableFlags_SizingFixedFit |
                               ImGuiTableFlags_NoHostExtendX;
  if (ImGui::BeginTable(IM_UNIQUE_ID, 2, kTableFlags)) {
    ImGui::TableSetupColumn("name");
    ImGui::TableSetupColumn("value");
    ImGui::TableHeadersRow();

    for (auto &[name, value] : properties) {
      ImGui::TableNextRow();

      ImGui::TableSetColumnIndex(0);
      ImGui::Text(name.c_str());

      ImGui::TableSetColumnIndex(1);
      dirty |= inspect(IM_UNIQUE_ID, value);
    }
    ImGui::EndTable();
  }

  return dirty;
}
bool inspect(const char *name, gfx::Property::Value &value) {
  ImGui::PushID(&value);

  constexpr auto kBaseWidth = 50.0f;
  const auto dirty =
    std::visit(Overload{
                 [name](int32_t &v) {
                   ImGui::SetNextItemWidth(kBaseWidth);
                   return ImGui::DragInt(name, &v);
                 },
                 [name](uint32_t &v) {
                   ImGui::SetNextItemWidth(kBaseWidth);
                   return ImGui::DragScalar(name, ImGuiDataType_U32, &v);
                 },
                 [name](float &v) {
                   ImGui::SetNextItemWidth(kBaseWidth);
                   return ImGui::DragFloat(name, &v);
                 },
                 [name](glm::vec2 &v) {
                   ImGui::SetNextItemWidth(kBaseWidth *
                                           std::decay_t<decltype(v)>::length());
                   return ImGui::DragFloat2(name, glm::value_ptr(v));
                 },
                 [name](glm::vec4 &v) {
                   ImGui::SetNextItemWidth(kBaseWidth *
                                           std::decay_t<decltype(v)>::length());
                   return ImGui::ColorEdit4(name, glm::value_ptr(v),
                                            ImGuiColorEditFlags_HDR |
                                              ImGuiColorEditFlags_Float);
                 },
               },
               value);
  ImGui::PopID();
  return dirty;
}
