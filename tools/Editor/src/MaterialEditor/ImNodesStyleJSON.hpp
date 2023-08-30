#pragma once

#include "imnodes.h"
#include <filesystem>

bool save(const std::filesystem::path &, const ImNodesStyle &);
bool load(const std::filesystem::path &, ImNodesStyle &);
