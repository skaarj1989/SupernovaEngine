#pragma once

#include "NodeCommon.hpp"

struct PostProcessMasterNode final : NodeBase {
  VertexDescriptor fragColor;

  // ---

  static PostProcessMasterNode create(ShaderGraph &, VertexDescriptor parent);

  bool inspect(ShaderGraph &, int32_t id);
  [[nodiscard]] MasterNodeResult evaluate(MaterialGenerationContext &,
                                          int32_t id) const;

  template <class Archive> void serialize(Archive &archive) {
    archive(Serializer{fragColor});
  }
};
