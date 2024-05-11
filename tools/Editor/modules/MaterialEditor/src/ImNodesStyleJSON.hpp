#pragma once

#include <filesystem>

struct ImNodesStyle;

bool save(const std::filesystem::path &, const ImNodesStyle &);
bool load(const std::filesystem::path &, ImNodesStyle &);
