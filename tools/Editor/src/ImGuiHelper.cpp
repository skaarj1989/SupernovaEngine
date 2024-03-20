#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGuiHelper.hpp"
#include "os/FileSystem.hpp"
#include "imgui_internal.h"

namespace ImGui {

namespace {

void PushColorScheme(int32_t i) {
  // ImGui Demo Window -> Widgets/Vertical Sliders.
  const auto h = i / 7.0f;
  PushStyleColor(ImGuiCol_FrameBg, ImVec4{ImColor::HSV(h, 0.5f, 0.5f)});
  PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4{ImColor::HSV(h, 0.6f, 0.5f)});
  PushStyleColor(ImGuiCol_FrameBgActive, ImVec4{ImColor::HSV(h, 0.7f, 0.5f)});
  PushStyleColor(ImGuiCol_SliderGrab, ImVec4{ImColor::HSV(h, 0.9f, 0.9f)});
}
void PopColorScheme() { PopStyleColor(4); }

template <typename Func> bool ColoredScalar3(const char *label, Func callback) {
  bool changed{false};

  BeginGroup();
  PushID(label);

  constexpr auto kNumComponents = 3;

  PushMultiItemsWidths(kNumComponents, CalcItemWidth());
  for (auto i = 0; i < kNumComponents; ++i) {
    PushID(i);
    if (i > 0) SameLine();
    PushColorScheme(i * 2);
    changed |= callback(i);
    PopColorScheme();
    PopID();
    PopItemWidth();
  }
  PopID();

  if (const char *labelEnd = FindRenderedTextEnd(label); label != labelEnd) {
    SameLine(0, GetStyle().ItemInnerSpacing.x);
    TextEx(label, labelEnd);
  }

  EndGroup();

  return changed;
}

} // namespace

ImGuiID GetLastItemID() { return GetCurrentContext()->LastItemData.ID; }

glm::vec2 GetCurrentWindowCenter() {
  return GetWindowPos() + (GetWindowSize() * 0.5f);
}

void CenterNextWindow(ImGuiCond cond) {
  const auto center = GetMainViewport()->GetCenter();
  SetNextWindowPos(center, cond, {0.5f, 0.5f});
}

void ShowTooltip(const std::string_view s, ImVec2 padding) {
  PushStyleVar(ImGuiStyleVar_WindowPadding, padding);
  SetTooltip(s.data());
  PopStyleVar();
}

bool CheckboxN(bool *data, int32_t components) {
  assert(data);

  auto result = false;
  for (auto i = 0; i < components; ++i) {
    PushID(i);
    result |= Checkbox(IM_UNIQUE_ID, data);
    PopID();
    SameLine();
    ++data;
  }
  return result;
}

bool SliderAngle3(const char *label, glm::vec3 &v) {
  return ColoredScalar3(
    label, [&v](auto i) { return SliderAngle(IM_UNIQUE_ID, &v[i]); });
}
bool InputFloat3(const char *label, glm::vec3 &v, const char *format,
                 ImGuiInputTextFlags flags) {
  return ColoredScalar3(label, [&](auto i) {
    return InputScalar(IM_UNIQUE_ID, ImGuiDataType_Float, &v[i], nullptr,
                       nullptr, format, flags);
  });
}
bool DragFloat3(const char *label, glm::vec3 &v, float speed, float min,
                float max, const char *format, ImGuiSliderFlags flags) {
  return ColoredScalar3(label, [&](auto i) {
    return DragFloat(IM_UNIQUE_ID, &v[i], speed, min, max, format, flags);
  });
}

void PrintPath(const std::filesystem::path &p) {
  const auto rp = os::FileSystem::relativeToRoot(p);
  BulletText("Path: %s", rp ? rp->generic_string().c_str() : "(empty)");
}

} // namespace ImGui

// https://github.com/ocornut/imgui/issues/7139#issuecomment-1861085213
ImGuiForceItemWidth ::ImGuiForceItemWidth(float width)
    : m_value{GImGui->CurrentWindow->WorkRect.Max.x} {
  ImGui::SetNextItemWidth(width);
  m_backup = m_value;
  m_value = GImGui->CurrentWindow->DC.CursorPos.x + ImGui::CalcItemWidth();
}
ImGuiForceItemWidth ::~ImGuiForceItemWidth() { m_value = m_backup; }

int32_t blockFileSystemForbiddenCharacters(ImGuiInputTextCallbackData *data) {
  if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
    if (os::FileSystem::getForbiddenCharacters().contains(data->EventChar)) {
      return -1;
    }
  }
  return 0;
}
