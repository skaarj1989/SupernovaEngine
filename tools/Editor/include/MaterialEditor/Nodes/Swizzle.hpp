#pragma once

#include "NodeCommon.hpp"

struct SwizzleNode final : NodeBase {
  VertexDescriptor input;

  std::string mask;

  // ---

  static SwizzleNode create(ShaderGraph &, VertexDescriptor parent);
  SwizzleNode clone(ShaderGraph &, VertexDescriptor parent) const;

  void remove(ShaderGraph &);

  bool inspect(ShaderGraph &, int32_t id);
  [[nodiscard]] NodeResult evaluate(MaterialGenerationContext &,
                                    int32_t id) const;

  template <class Archive> void serialize(Archive &archive) {
    archive(Serializer{input}, mask);
  }
};
