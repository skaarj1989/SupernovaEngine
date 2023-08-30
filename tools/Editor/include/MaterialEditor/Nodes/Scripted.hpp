#pragma once

#include "NodeCommon.hpp"
#include "MaterialEditor/ScriptedFunctionData.hpp"

// A node for a function with N input parameters and a single output.
struct ScriptedNode final : CustomizableNode {
  std::optional<uint32_t> guid;
  const ScriptedFunctionData *data{nullptr};

  // ---

  static ScriptedNode create(ShaderGraph &, VertexDescriptor parent,
                             std::pair<uint32_t, const ScriptedFunctionData *>);
  ScriptedNode clone(ShaderGraph &, VertexDescriptor parent) const;

  bool inspect(ShaderGraph &, int32_t id);
  [[nodiscard]] NodeResult evaluate(MaterialGenerationContext &,
                                    int32_t id) const;

  [[nodiscard]] const char *toString() const;
  bool setFunction(const ScriptedFunctions &);

  template <class Archive> void save(Archive &archive) const {
    assert(guid);
    archive(guid);
    CustomizableNode::save(archive);
  }
  template <class Archive> void load(Archive &archive) {
    archive(guid);
    CustomizableNode::load(archive);
  }
};
