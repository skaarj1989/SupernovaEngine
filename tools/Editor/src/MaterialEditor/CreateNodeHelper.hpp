#pragma once

#include "MaterialEditor/ShaderGraph.hpp"

template <typename Factory>
auto createNode(ShaderGraph &g, const Factory &factory) {
  const auto vd = g.addVertex(std::nullopt, NodeFlags::Output);
  auto &vertexProp = g.getVertexProp(vd);
  vertexProp.variant = factory(g, vd);
  return IDPair{vd, vertexProp.id};
}
