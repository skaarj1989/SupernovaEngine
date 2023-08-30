#pragma once

#include "imgui.h"
#include <filesystem>

bool save(const std::filesystem::path &, const ImGuiStyle &);
bool load(const std::filesystem::path &, ImGuiStyle &);
