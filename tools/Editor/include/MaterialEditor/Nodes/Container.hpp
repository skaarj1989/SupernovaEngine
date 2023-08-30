#pragma once

#include "NodeCommon.hpp"

struct ContainerNode final : NodeBase {
  std::string name;
  std::vector<VertexDescriptor> outputs;

  // ---

  ContainerNode clone(ShaderGraph &, VertexDescriptor parent) const;

  void remove(ShaderGraph &);
  bool inspect(ShaderGraph &, int32_t id);

  template <class Archive> void save(Archive &archive) const {
    archive(name);

    archive(outputs.size());
    for (auto vd : outputs)
      archive(Serializer{vd});
  }
  template <class Archive> void load(Archive &archive) {
    archive(name);

    std::size_t numOutputs{0};
    archive(numOutputs);
    outputs.resize(numOutputs);

    for (auto &vd : outputs)
      archive(Serializer{vd});
  }
};
