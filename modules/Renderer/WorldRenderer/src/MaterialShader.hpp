#pragma once

#include "rhi/ShaderType.hpp"
#include "renderer/Material.hpp"
#include "ShaderCodeBuilder.hpp"

namespace gfx {

void noMaterial(ShaderCodeBuilder &);

enum class ReferenceFrame { Local, World, View, Clip };
struct ReferenceFrames {
  ReferenceFrame position;
  ReferenceFrame normal;
};
void setReferenceFrames(ShaderCodeBuilder &, ReferenceFrames);

void addMaterial(ShaderCodeBuilder &, const Material &, rhi::ShaderType,
                 VkDeviceSize minOffsetAlignment);

} // namespace gfx
