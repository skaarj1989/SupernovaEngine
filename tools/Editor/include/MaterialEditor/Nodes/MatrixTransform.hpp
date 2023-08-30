#pragma once

#include "NodeCommon.hpp"

struct MatrixTransformNode final : NodeBase {
  VertexDescriptor input;

  enum class Space { Local, World, View, Clip, NDC, COUNT };
  Space source{Space::Local};
  Space target{Space::World};

  // ---

  static MatrixTransformNode create(ShaderGraph &, VertexDescriptor parent);
  MatrixTransformNode clone(ShaderGraph &, VertexDescriptor parent) const;

  void remove(ShaderGraph &);

  bool inspect(ShaderGraph &, int32_t id);
  [[nodiscard]] NodeResult evaluate(MaterialGenerationContext &,
                                    int32_t id) const;

  template <class Archive> void serialize(Archive &archive) {
    archive(Serializer{input}, source, target);
  }
};
