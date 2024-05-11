#pragma once

#include "renderer/Material.hpp"
#include <filesystem>

bool exportMaterial(const std::filesystem::path &, const std::string_view name,
                    const gfx::Material::Blueprint &);
