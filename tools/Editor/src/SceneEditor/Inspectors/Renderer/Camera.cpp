#include "SceneEditor/SceneEditor.hpp"

#include "Inspectors/CameraInspector.hpp"
#include "Inspectors/SkyLightInspector.hpp"
#include "Inspectors/PostProcessEffectInspector.hpp"

#include "ImGuiHelper.hpp"
#include "ImGuiPopups.hpp"
#include "RenderSettings.hpp"
#include "RenderSystem.hpp"

namespace {

[[nodiscard]] auto toString(rhi::Extent2D extent) {
  return std::format("{}x{}", extent.width, extent.height);
}
[[nodiscard]] auto extentSelector() {
  std::optional<rhi::Extent2D> out;

  if (ImGui::BeginMenu("4:3")) {
    // clang-format off
    constexpr auto kItems = std::array{
      rhi::Extent2D{1920, 1440},
      rhi::Extent2D{1856, 1392},
      rhi::Extent2D{1600, 1200},
      rhi::Extent2D{1440, 1080},
      rhi::Extent2D{1400, 1050},
      rhi::Extent2D{1280, 960},
      rhi::Extent2D{1152, 864},
      rhi::Extent2D{1024, 768},
      rhi::Extent2D{960, 720},
      rhi::Extent2D{800, 600},
      rhi::Extent2D{640, 480},
      rhi::Extent2D{320, 240},
      rhi::Extent2D{256, 192},
      rhi::Extent2D{160, 120},
    };
    // clang-format on
    for (const auto &extent : kItems) {
      if (ImGui::MenuItem(toString(extent).c_str())) {
        out = extent;
      }
    }
    ImGui::EndMenu();
  }
  if (ImGui::BeginMenu("16:9")) {
    // clang-format off
    constexpr auto kItems = std::array{
      rhi::Extent2D{3840, 2160},
      rhi::Extent2D{2560, 1440},
      rhi::Extent2D{1920, 1080},
      rhi::Extent2D{1280, 720},
      rhi::Extent2D{854, 480},
      rhi::Extent2D{640, 360},
      rhi::Extent2D{426, 240},
    };
    // clang-format on
    for (const auto &extent : kItems) {
      if (ImGui::MenuItem(toString(extent).c_str())) {
        out = extent;
      }
    }
    ImGui::EndMenu();
  }

  return out;
}

} // namespace

void SceneEditor::_onInspect(entt::handle h, CameraComponent &c) const {
  auto &mainCamera = getMainCamera(*h.registry());
  if (auto b = mainCamera.e == h; ImGui::Checkbox("main", &b)) {
    mainCamera.e = b ? h.entity() : entt::null;
  }

  auto dirtyExtent = false;

  ImGui::SetNextItemWidth(100);
  if (ImGui::InputScalarN("extent", ImGuiDataType_U32, &c.extent, 2, nullptr,
                          nullptr, nullptr,
                          ImGuiInputTextFlags_EnterReturnsTrue)) {
    dirtyExtent = true;
  }
  std::optional<rhi::Extent2D> newExtent;
  attachPopup(IM_UNIQUE_ID, ImGuiMouseButton_Right,
              [&newExtent] { newExtent = extentSelector(); });
  if (newExtent) {
    c.extent = *newExtent;
    dirtyExtent = true;
  }
  if (dirtyExtent) {
    if (c.extent) {
      c.camera.setAspectRatio(c.extent.getAspectRatio());
    }
  }

  constexpr auto kTreeNodeFlags = ImGuiTreeNodeFlags_Bullet;

  if (ImGui::CollapsingHeader("camera", kTreeNodeFlags)) {
    ImGui::Frame([&c] { inspect(c.camera, false); });
  }
  if (ImGui::CollapsingHeader("renderSettings", kTreeNodeFlags)) {
    ImGui::Frame([&c] { showRenderSettings(c.renderSettings); });
  }
  if (ImGui::CollapsingHeader("skyLight", kTreeNodeFlags)) {
    ImGui::Frame([&c, &renderer = getRenderer(*h.registry())] {
      inspect(c.skyLight, renderer);
    });
  }
  if (ImGui::CollapsingHeader("postProcessEffects", kTreeNodeFlags)) {
    ImGui::Frame([&c] { inspectPostProcessEffects(c.postProcessEffects); });
  }
}
