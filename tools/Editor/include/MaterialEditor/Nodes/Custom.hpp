#pragma once

#include "NodeCommon.hpp"
#include "MaterialEditor/UserFunctionData.hpp"

struct CustomNode final : CustomizableNode {
  std::optional<std::size_t> hash;
  const UserFunctionData *data{nullptr};

  // ---

  static CustomNode create(ShaderGraph &, VertexDescriptor parent,
                           std::pair<std::size_t, const UserFunctionData *>);
  CustomNode clone(ShaderGraph &, VertexDescriptor parent) const;

  bool inspect(ShaderGraph &, int32_t id);
  [[nodiscard]] NodeResult evaluate(MaterialGenerationContext &,
                                    int32_t id) const;

  bool setFunction(const UserFunctions &);

  template <class Archive> void save(Archive &archive) const {
    assert(hash);
    archive(hash);
    CustomizableNode::save(archive);
  }
  template <class Archive> void load(Archive &archive) {
    archive(hash);
    CustomizableNode::load(archive);
  }
};
