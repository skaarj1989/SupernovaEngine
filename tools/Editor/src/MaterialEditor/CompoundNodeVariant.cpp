#include "MaterialEditor/CompoundNodeVariant.hpp"
#include "VisitorHelper.hpp"

const char *toString(const CompoundNodeVariant &variant) {
  return std::visit(
    Overload{
      [](const CustomNode &n) {
        return n.data ? n.data->name.c_str() : "Undefined";
      },
      [](const ScriptedNode &n) { return n.toString(); },
      [](const ArithmeticNode &n) { return toString(n.operation); },
      [](const AppendNode &) { return "Append"; },
      [](const VectorSplitterNode &) { return "VectorSplitter"; },
      [](const MatrixSplitterNode &) { return "MatrixSplitter"; },
      [](const SwizzleNode &) { return "Swizzle"; },
      [](const MatrixTransformNode &) { return "MatrixTransform"; },
      [](const TextureSamplingNode &) { return "TextureSampling"; },
    },
    variant);
}
