#pragma once

#include "assimp/material.h"
#include "SubMesh.hpp"

[[nodiscard]] offline::Material convert(const aiMaterial &, uint32_t index);

std::size_t exportMaterials(std::span<const offline::SubMesh>,
                            const std::filesystem::path &dir);
