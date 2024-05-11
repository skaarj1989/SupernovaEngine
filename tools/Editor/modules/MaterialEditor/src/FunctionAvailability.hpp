#pragma once

#include "renderer/Material.hpp"

[[nodiscard]] bool vertexShaderOnly(const gfx::Material::Surface *,
                                    const rhi::ShaderType);
[[nodiscard]] bool fragmentShaderOnly(const gfx::Material::Surface *,
                                      const rhi::ShaderType);

[[nodiscard]] bool surfaceOnly(const gfx::Material::Surface *,
                               const rhi::ShaderType);
[[nodiscard]] bool postProcessOnly(const gfx::Material::Surface *,
                                   const rhi::ShaderType);
