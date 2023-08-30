#pragma once

#include "NodeCommon.hpp"

struct ArithmeticNode final : NodeBase {
  struct Input {
    VertexDescriptor lhs;
    VertexDescriptor rhs;

    template <class Archive> void serialize(Archive &archive) {
      archive(Serializer{lhs}, Serializer{rhs});
    }
  };
  Input input;

  enum class Operation { Add, Subtract, Multiply, Divide, COUNT };
  Operation operation{Operation::Add};

  // ---

  static ArithmeticNode create(ShaderGraph &, VertexDescriptor parent,
                               Operation);
  ArithmeticNode clone(ShaderGraph &, VertexDescriptor parent) const;

  void remove(ShaderGraph &);

  bool inspect(ShaderGraph &, int32_t id);
  [[nodiscard]] NodeResult evaluate(MaterialGenerationContext &,
                                    int32_t id) const;

  template <class Archive> void serialize(Archive &archive) {
    archive(input, operation);
  }
};

[[nodiscard]] const char *toString(ArithmeticNode::Operation);
