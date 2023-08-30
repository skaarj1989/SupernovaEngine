#pragma once

#include "NodeCommon.hpp"

struct AppendNode final : NodeBase {
  struct Input {
    VertexDescriptor a;
    VertexDescriptor b;

    template <class Archive> void serialize(Archive &archive) {
      archive(Serializer{a}, Serializer{b});
    }
  };
  Input input;

  // ---

  static AppendNode create(ShaderGraph &, VertexDescriptor parent);
  AppendNode clone(ShaderGraph &, VertexDescriptor parent) const;

  void remove(ShaderGraph &);

  bool inspect(ShaderGraph &, int32_t id);
  [[nodiscard]] NodeResult evaluate(MaterialGenerationContext &,
                                    int32_t id) const;

  template <class Archive> void serialize(Archive &archive) { archive(input); }
};
