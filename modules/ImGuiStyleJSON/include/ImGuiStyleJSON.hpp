#pragma once

#include <filesystem>

struct ImGuiStyle;

bool save(const std::filesystem::path &, const ImGuiStyle &);
bool load(const std::filesystem::path &, ImGuiStyle &);
