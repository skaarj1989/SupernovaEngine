#pragma once

#include "ImGuiHelper.hpp"
#include <array>
#include <bitset>
#include <optional>

enum class ModalButton { Ok, Yes, No, Cancel };

[[nodiscard]] const char *toString(ModalButton);

enum class ModalButtons {
  Ok = 1 << 0,
  Yes = 1 << 1,
  No = 1 << 2,
  Cancel = 1 << 3,
};
template <> struct has_flags<ModalButtons> : std::true_type {};

template <ModalButtons _Buttons> constexpr auto getButtons() {
  constexpr auto n = std::to_underlying(_Buttons);
  constexpr auto kNumButtons = std::bitset<sizeof(int32_t)>(n).count();

  std::array<ModalButton, kNumButtons> buttons;
  auto ptr = buttons.begin();

  constexpr auto kMaxNumButtons = 4;
  for (auto i = 0; i < kMaxNumButtons; ++i) {
    if (bool(n & (1 << i))) *(ptr++) = static_cast<ModalButton>(i);
  }
  return buttons;
}

template <ModalButtons _Buttons = ModalButtons::Ok, typename Func>
std::optional<ModalButton> showModal(const char *name, Func callback) {
  std::optional<ModalButton> result;

  ImGui::CenterNextWindow();
  if (ImGui::BeginPopupModal(name, nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {
    callback();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    for (auto i = 0; const auto button : getButtons<_Buttons>()) {
      if (i > 0) ImGui::SameLine();
      if (ImGui::Button(toString(button))) {
        result = button;
        ImGui::CloseCurrentPopup();
      }
      ++i;
    }

    ImGui::EndPopup();
  }
  return result;
}

template <ModalButtons _Buttons = ModalButtons::Ok>
auto showMessageBox(const char *name, const char *text) {
  return showModal<_Buttons>(name, [text] { ImGui::Text(text); });
}
