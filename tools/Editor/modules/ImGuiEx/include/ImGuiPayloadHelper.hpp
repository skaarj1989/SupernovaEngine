#pragma once

#include "entt/core/fwd.hpp" // id_type
#include "imgui.h"           // ImGuiPayload->Data
#include <bit>               // bit_cast
#include <filesystem>        // path

namespace ImGui {

bool StorePointer(const char *type, const void *ptr);

template <typename T> auto ExtractPointer(const ImGuiPayload *payload) {
  const auto address = *static_cast<std::uintptr_t *>(payload->Data);
  return std::bit_cast<T *>(address);
}

[[nodiscard]] entt::id_type ExtractId(const ImGuiPayload &);

[[nodiscard]] std::string_view ExtractString(const ImGuiPayload &);
/*
  @param Payload must be a string (path relative to the project file).
  @return Absolute path (relative to the project root).
*/
[[nodiscard]] std::filesystem::path ExtractPath(const ImGuiPayload &);

} // namespace ImGui
