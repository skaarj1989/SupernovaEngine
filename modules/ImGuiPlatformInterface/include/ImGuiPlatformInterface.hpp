#pragma once

#include <optional>

namespace os {
class Window;
enum class KeyCode;
} // namespace os

enum ImGuiKey;

[[nodiscard]] std::optional<ImGuiKey> remapKeyCode(const os::KeyCode);
[[nodiscard]] std::optional<ImGuiKey> getKeyModifier(const os::KeyCode);
void setupPlatformInterface(os::Window &mainWindow);
