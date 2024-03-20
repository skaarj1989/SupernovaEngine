#pragma once

class ShaderGraph;
struct ImNodesEditorContext;

template <class GraphT, typename NodesEditorContextT>
struct ShaderGraphStageView {
  GraphT *graph{nullptr};
  NodesEditorContextT *nodeEditorContext{nullptr};

  operator ShaderGraphStageView<const GraphT, NodesEditorContextT>()
    const noexcept {
    return {graph, nodeEditorContext};
  }
};

using ShaderGraphStageViewMutable =
  ShaderGraphStageView<ShaderGraph, ImNodesEditorContext>;
using ShaderGraphStageViewConst =
  ShaderGraphStageView<const ShaderGraph, ImNodesEditorContext>;
