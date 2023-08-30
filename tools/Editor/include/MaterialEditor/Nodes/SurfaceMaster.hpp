#pragma once

#include "NodeCommon.hpp"
#include "renderer/Material.hpp"

struct SurfaceMasterNode final : CustomizableNode {
  void remove(ShaderGraph &) = delete;

  static SurfaceMasterNode create(ShaderGraph &, VertexDescriptor parent);

  bool inspect(ShaderGraph &, int32_t id, const gfx::Material::Surface &);
  [[nodiscard]] MasterNodeResult evaluate(MaterialGenerationContext &,
                                          int32_t id) const;
};
