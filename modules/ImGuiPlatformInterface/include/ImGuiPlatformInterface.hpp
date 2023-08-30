#pragma once

#include "os/Window.hpp"

enum ImGuiKey;

[[nodiscard]] std::optional<ImGuiKey> remapKeyCode(os::KeyCode);
[[nodiscard]] std::optional<ImGuiKey> getKeyModifier(os::KeyCode);
void setupPlatformInterface(os::Window &mainWindow);
