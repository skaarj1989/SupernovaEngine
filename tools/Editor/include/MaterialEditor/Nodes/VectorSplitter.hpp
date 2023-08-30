#pragma once

#include "NodeCommon.hpp"

struct VectorSplitterNode final : NodeBase {
  std::optional<VertexDescriptor> input;

  struct Output {
    VertexDescriptor xyz;

    VertexDescriptor x;
    VertexDescriptor y;
    VertexDescriptor z;
    VertexDescriptor w;

    template <class Archive> void serialize(Archive &archive) {
      // clang-format off
      archive(
        Serializer{xyz},
        Serializer{x},
        Serializer{y},
        Serializer{z},
        Serializer{w}
      );
      // clang-format on
    }
  };
  Output output;

  // ---

  static VectorSplitterNode create(ShaderGraph &, VertexDescriptor parent);
  VectorSplitterNode clone(ShaderGraph &, VertexDescriptor parent) const;

  void remove(ShaderGraph &);

  bool inspect(ShaderGraph &, int32_t id);
  [[nodiscard]] NodeResult evaluate(MaterialGenerationContext &,
                                    int32_t id) const;

  template <class Archive> void serialize(Archive &archive) {
    archive(input, output);
  }
};
