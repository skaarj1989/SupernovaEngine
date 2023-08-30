#pragma once

#include "rhi/ShaderType.hpp"
#include "renderer/Material.hpp"

[[nodiscard]] bool vertexShaderOnly(const rhi::ShaderType,
                                    const gfx::Material::Blueprint &);
[[nodiscard]] bool fragmentShaderOnly(const rhi::ShaderType,
                                      const gfx::Material::Blueprint &);

[[nodiscard]] bool surfaceOnly(const rhi::ShaderType,
                               const gfx::Material::Blueprint &);
[[nodiscard]] bool postProcessOnly(const rhi::ShaderType,
                                   const gfx::Material::Blueprint &);
