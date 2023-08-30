#pragma once

#include "NodeCommon.hpp"

struct TextureSamplingNode final : NodeBase {
  struct Input {
    VertexDescriptor sampler;
    VertexDescriptor uv;
    struct LOD {
      VertexDescriptor vd{nullptr};
      bool enabled{false};

      template <class Archive> void serialize(Archive &archive) {
        archive(Serializer{vd}, enabled);
      }
    };
    LOD lod;

    template <class Archive> void serialize(Archive &archive) {
      archive(Serializer{sampler}, Serializer{uv}, lod);
    }
  };
  Input input;

  VertexDescriptor split; // VectorSplitterNode

  // ---

  static TextureSamplingNode create(ShaderGraph &, VertexDescriptor parent);
  TextureSamplingNode clone(ShaderGraph &, VertexDescriptor parent) const;

  void remove(ShaderGraph &);

  bool inspect(ShaderGraph &, int32_t id);
  [[nodiscard]] NodeResult evaluate(MaterialGenerationContext &,
                                    int32_t id) const;

  template <class Archive> void serialize(Archive &archive) {
    archive(input, Serializer{split});
  }
};
