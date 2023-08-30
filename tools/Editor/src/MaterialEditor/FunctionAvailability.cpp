#include "MaterialEditor/FunctionAvailability.hpp"

bool vertexShaderOnly(const rhi::ShaderType shaderType,
                      const gfx::Material::Blueprint &) {
  return shaderType == rhi::ShaderType::Vertex;
}
bool fragmentShaderOnly(const rhi::ShaderType shaderType,
                        const gfx::Material::Blueprint &) {
  return shaderType == rhi::ShaderType::Fragment;
}

bool surfaceOnly(const rhi::ShaderType,
                 const gfx::Material::Blueprint &blueprint) {
  return blueprint.surface.has_value();
}
bool postProcessOnly(const rhi::ShaderType,
                     const gfx::Material::Blueprint &blueprint) {
  return !blueprint.surface.has_value();
}
