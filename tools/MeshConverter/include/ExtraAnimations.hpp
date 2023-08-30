#pragma once

#include "assimp/scene.h"
#include "assimp/Importer.hpp"

#include <span>
#include <vector>
#include <filesystem>

[[nodiscard]] std::vector<aiAnimation *>
loadExtraAnimations(Assimp::Importer &, const std::filesystem::path &dir,
                    std::span<const std::string> names);

void copyAnimations(aiScene *dst, std::span<aiAnimation *>);
