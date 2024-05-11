#pragma once

#include "ShaderGraphCommon.hpp"

template <typename Func>
void traverse(auto &graph, const VertexDescriptor root, Func visitor) {
  assert(root != nullptr);

  std::stack<VertexDescriptor> stack;
  stack.push(root);

  while (!stack.empty()) {
    const auto current = stack.top();
    stack.pop();

    std::invoke(visitor, current);

    for (const auto vd :
         boost::make_iterator_range(boost::adjacent_vertices(current, graph))) {
      stack.push(vd);
    }
  }
}
