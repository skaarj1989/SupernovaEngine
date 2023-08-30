#pragma once

#include "MaterialEditor/ShaderToken.hpp"
#include "MaterialEditor/VertexDescriptorSerializer.hpp"
#include <expected>

class ShaderGraph;
struct MaterialGenerationContext;

struct NodeBase {
  using Serializer = VertexDescriptorSerializer;
};

struct CustomizableNode : NodeBase {
  std::vector<VertexDescriptor> inputs;

  // ---

  void remove(ShaderGraph &);

  template <class Archive> void save(Archive &archive) const {
    archive(inputs.size());
    for (auto vd : inputs)
      archive(Serializer{vd});
  }
  template <class Archive> void load(Archive &archive) {
    std::size_t numInputs{0};
    archive(numInputs);
    inputs.resize(numInputs);

    for (auto &vd : inputs)
      archive(Serializer{vd});
  }
};

using NodeResult = std::expected<ShaderToken, std::string>;
using MasterNodeResult = std::expected<std::monostate, std::string>;
