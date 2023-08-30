#include "ImGuiPayloadHelper.hpp"
#include "os/FileSystem.hpp"

namespace ImGui {

bool StorePointer(const char *type, const void *ptr) {
  const auto address = std::bit_cast<std::uintptr_t>(ptr);
  return SetDragDropPayload(type, &address, sizeof(std::uintptr_t));
}

entt::id_type ExtractId(const ImGuiPayload &payload) {
  return *static_cast<entt::id_type *>(payload.Data);
}

std::string_view ExtractString(const ImGuiPayload &payload) {
  return {
    static_cast<const char *>(payload.Data),
    static_cast<std::size_t>(payload.DataSize),
  };
}
std::filesystem::path ExtractPath(const ImGuiPayload &payload) {
  return os::FileSystem::getRoot() / ExtractString(payload);
}

} // namespace ImGui
