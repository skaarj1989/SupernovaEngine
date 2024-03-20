#pragma once

#include "Nodes/NodeVisitor.hpp"
#include "ShaderGraphCommon.hpp"
#include "ShaderGraphStageView.hpp"
#include "ScriptedFunction.hpp"
#include "UserFunction.hpp"

class PathMap;

class NodePatcherVisitor : public NodeVisitor {
public:
  NodePatcherVisitor(const PathMap *, const ScriptedFunctions *,
                     const UserFunctions *);

  void patch(NodeBase &);

private:
  void visit(EmbeddedNode<TextureParam> &) override;
  void visit(ScriptedNode &) override;
  void visit(CustomNode &) override;

private:
  const PathMap *m_texturePaths{nullptr};
  const ScriptedFunctions *m_scriptedFunctions{nullptr};
  const UserFunctions *m_userFunctions{nullptr};
};
