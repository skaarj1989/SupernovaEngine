#include "Inspectors/CollisionLayerInspector.hpp"
#include "ImGuiPopups.hpp"
#include "imgui_internal.h" // {Push/Pop}ItemFlag
#include <bitset>
#include <format>

namespace {

bool inspect(const char *label, uint8_t &mask) {
  auto dirty = false;

  const auto numLayers = std::bitset<CollisionLayer::kNumGroups>{mask}.count();
  if (ImGui::BeginCombo(label, std::format("{} layer(s)", numLayers).c_str())) {
    ImGui::PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
    for (auto i = 0; i < CollisionLayer::kNumGroups; ++i) {
      ImGui::PushID(i);
      const auto value = 1 << i;
      const auto enabled = (mask & value) == value;
      if (ImGui::MenuItem(std::format("Layer {}", i).c_str(), nullptr,
                          enabled)) {
        if (enabled) {
          mask &= ~value;
        } else {
          mask |= value;
        }
        dirty = true;
      }
      ImGui::PopID();
    }
    ImGui::PopItemFlag();

    ImGui::EndCombo();
  }
  attachPopup(nullptr, ImGuiMouseButton_Right, [&mask, &dirty] {
    if (ImGui::MenuItem("Select all")) {
      mask = ~0;
      dirty = true;
    }
    if (ImGui::MenuItem("Clear")) {
      mask = 0;
      dirty = true;
    }
  });

  return dirty;
}

} // namespace

bool inspect(CollisionLayer &layer) {
  auto dirty = false;

  constexpr auto kMinGroup = 0u;
  constexpr auto kMaxGroup = CollisionLayer::kNumGroups - 1u;
  dirty |= ImGui::SliderScalar("group", ImGuiDataType_U8, &layer.group,
                               &kMinGroup, &kMaxGroup);
  dirty |= inspect("mask", layer.mask);

  return dirty;
}
