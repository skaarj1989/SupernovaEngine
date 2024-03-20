#include "MaterialEditor/NodePatcherVisitor.hpp"
#include "Services.hpp"
#include "MaterialEditor/PathMap.hpp"
#include "MaterialEditor/TextureParam.hpp"
#include "MaterialEditor/Nodes/Embedded.hpp"
#include "MaterialEditor/Nodes/Scripted.hpp"
#include "MaterialEditor/Nodes/Custom.hpp"

NodePatcherVisitor::NodePatcherVisitor(
  const PathMap *paths, const ScriptedFunctions *scriptedFunctions,
  const UserFunctions *userFunctions)
    : m_texturePaths{paths}, m_scriptedFunctions{scriptedFunctions},
      m_userFunctions{userFunctions} {}

void NodePatcherVisitor::patch(NodeBase &node) { node.accept(*this); }

void NodePatcherVisitor::visit(EmbeddedNode<TextureParam> &node) {
  if (const auto p = m_texturePaths->get(node.getOrigin(), node.vertex.id); p) {
    node.value.texture =
      Services::Resources::Textures::value().load(*p).handle();
  }
}
void NodePatcherVisitor::visit(ScriptedNode &node) {
  if (const auto it = m_scriptedFunctions->find(node.scriptedFunction.id);
      it != m_scriptedFunctions->cend()) {
    node.scriptedFunction.data = it->second.get();
  }
}
void NodePatcherVisitor::visit(CustomNode &node) {
  if (const auto it = m_userFunctions->find(node.userFunction.id);
      it != m_userFunctions->cend()) {
    node.userFunction.data = it->second.get();
  }
}
