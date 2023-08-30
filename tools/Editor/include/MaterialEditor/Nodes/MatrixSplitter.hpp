#pragma once

#include "NodeCommon.hpp"

struct MatrixSplitterNode final : NodeBase {
  VertexDescriptor input;

  struct Output {
    VertexDescriptor column0;
    VertexDescriptor column1;
    VertexDescriptor column2;
    VertexDescriptor column3;

    template <class Archive> void serialize(Archive &archive) {
      // clang-format off
      archive(
        Serializer{column0},
        Serializer{column1},
        Serializer{column2},
        Serializer{column3}
      );
      // clang-format on
    }
  };
  Output output;

  // ---

  static MatrixSplitterNode create(ShaderGraph &, VertexDescriptor parent);
  MatrixSplitterNode clone(ShaderGraph &, VertexDescriptor parent) const;

  void remove(ShaderGraph &);

  bool inspect(ShaderGraph &, int32_t id);
  [[nodiscard]] NodeResult evaluate(MaterialGenerationContext &,
                                    int32_t id) const;

  template <class Archive> void serialize(Archive &archive) {
    archive(Serializer{input}, output);
  }
};
