#include "TexturePreview.hpp"
#include "rhi/Texture.hpp"
#include "ImGuiPopups.hpp"
#include "imgui_internal.h" // ImRect
#include "glm/common.hpp"   // floor, mix

namespace {

[[nodiscard]] auto getLocalRect(const ImRect windowRect,
                                const glm::vec2 overlaySize,
                                const glm::vec2 offset,
                                const glm::vec2 origin) {
  constexpr glm::vec2 kBorder{0, 0};
  const auto topLeft = glm::vec2{windowRect.Min} + offset + kBorder;
  const auto bottomRight =
    glm::vec2{windowRect.Max} - offset - kBorder - overlaySize;

  const auto min = glm::floor(glm::mix(topLeft, bottomRight, origin));
  const auto max = min + overlaySize;
  return ImRect{min, max};
}

[[nodiscard]] bool canPreview(const rhi::Texture &texture) {
  return texture &&
         texture.getImageLayout() == rhi::ImageLayout::ShaderReadOnly;
}

} // namespace

void overlay(rhi::Texture *texture, const glm::vec2 size) {
  static constexpr auto kTopLeft = glm::vec2{0, 0};
  static constexpr auto kTopRight = glm::vec2{1, 0};
  static constexpr auto kBottomRight = glm::vec2{1, 1};
  static constexpr auto kBottomLeft = glm::vec2{0, 1};

  static auto origin = kBottomRight;

  constexpr auto kBorder = 4.0f;
  auto rect = getLocalRect({{0, 0}, ImGui::GetWindowSize()}, size,
                           glm::vec2{kBorder} * 0.5f, origin);
  rect.Translate(ImGui::GetWindowPos()); // Window local -> global.

  ImGui::SetNextWindowPos(rect.Min);
  if (ImGui::BeginChild(ImGui::GetID(IM_UNIQUE_ID), size)) {
    preview(texture, ImGui::GetContentRegionAvail());
    attachPopup(IM_UNIQUE_ID, ImGuiMouseButton_Right, [] {
      if (ImGui::MenuItem("top-left")) origin = kTopLeft;
      if (ImGui::MenuItem("top-right")) origin = kTopRight;
      if (ImGui::MenuItem("bottom-right")) origin = kBottomRight;
      if (ImGui::MenuItem("bottom-left")) origin = kBottomLeft;
    });
  }
  ImGui::EndChild();

  // The border of ImGui ChildWindow is too thick.
  constexpr auto kBorderColor = IM_COL32(255, 255, 255, 255);
  ImGui::GetWindowDrawList()->AddRect(rect.Min, rect.Max, kBorderColor, 0.0f, 0,
                                      kBorder);
}
void preview(rhi::Texture *texture, const glm::vec2 size) {
  if (texture && canPreview(*texture)) {
    ImGui::Image(texture, size);
  } else {
    ImGui::BeginDisabled(true);
    ImGui::Button("(empty)", size);
    ImGui::EndDisabled();
  }
}
