#pragma once

#include "rhi/ShaderType.hpp"

class ShaderCodeBuilder;

namespace gfx {

class Material;

void noMaterial(ShaderCodeBuilder &);

enum class ReferenceFrame { Local, World, View, Clip };
struct ReferenceFrames {
  ReferenceFrame position;
  ReferenceFrame normal;
};
void setReferenceFrames(ShaderCodeBuilder &, const ReferenceFrames);

void addMaterial(ShaderCodeBuilder &, const Material &, const rhi::ShaderType,
                 const VkDeviceSize minOffsetAlignment);

} // namespace gfx
