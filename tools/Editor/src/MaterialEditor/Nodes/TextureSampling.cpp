#include "MaterialEditor/Nodes/TextureSampling.hpp"
#include "Utility.hpp"

namespace {

struct InputInfo {
  const char *name;
  TransientVariant value;
};
const auto kInputs = std::array{
  InputInfo{"sampler", TextureParam{}},
  InputInfo{"uv", Attribute::TexCoord0},
  InputInfo{"lod", 0.0f},
};

} // namespace

//
// TextureSamplingNode class:
//

TextureSamplingNode::TextureSamplingNode(ShaderGraph &g, const IDPair vertex)
    : CompoundNode{g, vertex, Flags::Output} {
  auto hint = vertex.id + 1;
  inputs.reserve(kInputs.size());
  for (const auto &[name, value] : kInputs) {
    auto *node = createNode(g, hint++, value);
    node->label = name;
    node->flags = Flags::Internal | Flags::Input;
    inputs.emplace_back(node->vertex);
  }
}

std::unique_ptr<NodeBase>
TextureSamplingNode::clone(const IDPair cloned) const {
  auto node = std::make_unique<TextureSamplingNode>();
  node->useLOD = useLOD;
  cloneChildren(*this, *node, cloned.id + 1);
  return node;
}

std::string TextureSamplingNode::toString() const { return "TextureSampling"; }
