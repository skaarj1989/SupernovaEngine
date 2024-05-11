#include "FunctionAvailability.hpp"

bool vertexShaderOnly(const gfx::Material::Surface *,
                      const rhi::ShaderType shaderType) {
  return shaderType == rhi::ShaderType::Vertex;
}
bool fragmentShaderOnly(const gfx::Material::Surface *,
                        const rhi::ShaderType shaderType) {
  return shaderType == rhi::ShaderType::Fragment;
}

bool surfaceOnly(const gfx::Material::Surface *surface, const rhi::ShaderType) {
  return surface != nullptr;
}
bool postProcessOnly(const gfx::Material::Surface *surface,
                     const rhi::ShaderType) {
  return surface == nullptr;
}
