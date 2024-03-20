#include "GraphCommands.hpp"
#include "os/FileSystem.hpp"

#include "MaterialEditor/PropertyVariant.hpp"
#include "MaterialEditor/TextureParam.hpp"
#include "MaterialEditor/Nodes/Embedded.hpp"

#include "MaterialEditor/ShaderGraph.hpp"
#include "MaterialEditor/NodeFactoryRegistry.hpp"

#include "MaterialEditor/ImNodesHelper.hpp"
#include "imnodes_internal.h" // GImNodes

#include "spdlog/logger.h"

//
// CreateNodeCommand class:
//

CreateNodeCommand::CreateNodeCommand(const ShaderGraphStageViewMutable view,
                                     const NodeFactoryRegistry &registry,
                                     const FactoryID factoryId,
                                     const glm::vec2 position)
    : MutateGraphCommand{view}, m_registry{&registry}, m_factoryId{factoryId} {
  // Screen space -> Grid space
  m_position = position - glm::vec2{GImNodes->CanvasOriginScreenSpace} -
               glm::vec2{m_view.nodeEditorContext->Panning};
}

bool CreateNodeCommand::execute(const Context &ctx) {
  m_id = m_registry->create(m_factoryId, *m_view.graph, m_id).id;

  ImNodes::EditorContextSet(m_view.nodeEditorContext);
  ImNodes::SetNodeGridSpacePos(*m_id, m_position);
  ImNodes::ClearNodeSelection();
  ImNodes::SelectNode(*m_id);

  ctx.logger.info("[" __FUNCTION__ "][node_id={}] Created (factory_id={})",
                  *m_id, m_factoryId);
  return true;
}
bool CreateNodeCommand::undo(const Context &ctx) {
  if (const auto vd = m_view.graph->findVertex(*m_id); vd) {
    ImNodes::EditorContextSet(m_view.nodeEditorContext);
    ImNodes::ClearNodeSelection();

    m_view.graph->remove(vd);
    ctx.logger.info("[" __FUNCTION__ "][node_id={}] Removed", *m_id);
    return true;
  } else {
    ctx.logger.error("[" __FUNCTION__ "][node_id={}] Not Found!", *m_id);
    return true;
  }
}

//
// RemoveNodeCommand class:
//

RemoveNodeCommand::RemoveNodeCommand(const ShaderGraphStageViewMutable view,
                                     const VertexID id)
    : MutateGraphCommand{view}, m_id{id} {}

bool RemoveNodeCommand::execute(const Context &ctx) {
  auto [g, nec] = m_view;
  if (const auto vd = g->findVertex(m_id); vd) {
    ImNodes::EditorContextSet(nec);
    m_position = ImNodes::GetNodeGridSpacePos(m_id);
    ImNodes::ClearNodeSelection();

    m_memento = g->makeSnapshot(vd);
    // At this point the node should not have any in/out edges.
    g->remove(vd);
    ctx.logger.info("[" __FUNCTION__ "][node_id={}] Removed", m_id);
    return true;
  } else {
    ctx.logger.error("[" __FUNCTION__ "][node_id={}] Not Found!", m_id);
    return false;
  }
}
bool RemoveNodeCommand::undo(const Context &ctx) {
  m_view.graph->loadSnapshot(m_memento);
  ctx.logger.info("[" __FUNCTION__ "][node_id={}] Restored", m_id);

  ImNodes::EditorContextSet(m_view.nodeEditorContext);
  ImNodes::SetNodeGridSpacePos(m_id, m_position);
  ImNodes::SelectNode(m_id);
  return true;
}

//
// CloneNodeCommand class:
//

CloneNodeCommand::CloneNodeCommand(const ShaderGraphStageViewMutable view,
                                   const VertexID sourceId,
                                   const glm::vec2 position)
    : MutateGraphCommand{view}, m_sourceId{sourceId}, m_position{position} {
  ImNodes::EditorContextSet(m_view.nodeEditorContext);
  m_position = ImNodes::GetNodeGridSpacePos(m_sourceId);
  m_position += 32.0f;
}

bool CloneNodeCommand::execute(const Context &ctx) {
  const auto [g, nec] = m_view;
  if (const auto sourceVd = g->findVertex(m_sourceId); sourceVd) {
    const auto vertex = g->clone(sourceVd, m_id);
    m_id = vertex.id;
    ctx.logger.info(
      "[" __FUNCTION__ "][node_id={}] Created (source_node_id={})", *m_id,
      m_sourceId);

    ImNodes::EditorContextSet(nec);
    ImNodes::SetNodeGridSpacePos(*m_id, m_position);
    ImNodes::ClearNodeSelection();
    ImNodes::SelectNode(*m_id);
    return true;
  } else {
    ctx.logger.error("[" __FUNCTION__ "][source_node_id={}] Not Found!",
                     m_sourceId);
    return false;
  }
}
bool CloneNodeCommand::undo(const Context &ctx) {
  if (const auto vd = m_view.graph->findVertex(*m_id); vd) {
    ImNodes::EditorContextSet(m_view.nodeEditorContext);
    ImNodes::ClearNodeSelection();

    m_view.graph->remove(vd);
    ctx.logger.info("[" __FUNCTION__ "][node_id={}] Removed", *m_id);
    return true;
  } else {
    ctx.logger.error("[" __FUNCTION__ "][node_id={}] Not Found!", *m_id);
    return false;
  }
}
//
// RenameNodeCommand class:
//

RenameNodeCommand::RenameNodeCommand(ShaderGraph &g, const VertexID id,
                                     const std::string &name)
    : MutateGraphCommand{{&g, nullptr}}, m_id{id}, m_name{name} {}

bool RenameNodeCommand::execute(const Context &ctx) {
  if (auto *node = m_view.graph->findNode(m_id); node) {
    if (auto previousLabel = node->setLabel(m_name); previousLabel) {
      ctx.logger.info("[" __FUNCTION__ "][node_id={}] '{}'->'{}'", m_id, m_name,
                      node->label);
      m_name = std::move(*previousLabel);
      return true;
    }
  } else {
    ctx.logger.error("[" __FUNCTION__ "][node_id={}] Not Found!", m_id);
  }
  return false;
}
bool RenameNodeCommand::undo(const Context &ctx) { return execute(ctx); }

//
// CreateLinkCommand class:
//

CreateLinkCommand::CreateLinkCommand(ShaderGraph &g,
                                     const ConnectedIDs connection)
    : MutateGraphCommand{{&g, nullptr}}, m_connection{connection} {}

bool CreateLinkCommand::execute(const Context &ctx) {
  const auto [g, _] = m_view;
  const auto c = transform(*g, m_connection);
  assert(c.isValid());

  g->removeOutEdges(c.source.vd);
  if (const auto ed = g->link(EdgeType::External, c, m_id); ed) {
    m_id = g->getProperty(*ed).id;
    ctx.logger.info("[" __FUNCTION__ "][link_id={}] {}->{}", *m_id,
                    m_connection.second, m_connection.first);
    return true;
  } else {
    ctx.logger.warn("Link {}->{}, discarded. Cycles in a shader graph "
                    "are forbidden.",
                    m_connection.second, m_connection.first);
    return false;
  }
}
bool CreateLinkCommand::undo(const Context &ctx) {
  if (const auto ed = m_view.graph->findEdge(*m_id); ed) {
    m_view.graph->remove(*ed);
    ctx.logger.info("[" __FUNCTION__ "][link_id={}] Connected {}->{}", *m_id,
                    m_connection.second, m_connection.first);
    return true;
  } else {
    ctx.logger.error("[" __FUNCTION__ "][link_id={}, {}->{}] Not Found!", *m_id,
                     m_connection.second, m_connection.first);
    return false;
  }
}

//
// RemoveLinkCommand class:
//

RemoveLinkCommand::RemoveLinkCommand(ShaderGraph &g, const EdgeID id)
    : MutateGraphCommand{{&g, nullptr}}, m_id{id} {}

bool RemoveLinkCommand::execute(const Context &ctx) {
  auto &g = *m_view.graph;
  if (const auto ed = g.findEdge(m_id); ed) {
    const auto c = g.getConnection(*ed);
    assert(c.isValid());

    m_connection = c;
    g.remove(*ed);
    ctx.logger.info("[" __FUNCTION__ "][link_id={}] Disconnected {}-/>{}", m_id,
                    m_connection.second, m_connection.first);
    return true;
  } else {
    ctx.logger.error("[" __FUNCTION__ "][link_id={}, {}->{}] Not Found!", m_id,
                     m_connection.second, m_connection.first);
    return false;
  }
}
bool RemoveLinkCommand::undo(const Context &ctx) {
  auto &g = *m_view.graph;
  if (const auto c = transform(g, m_connection); c.isValid()) {
    g.link(EdgeType::External, c, m_id);
    ctx.logger.info("[" __FUNCTION__ "][link_id={}] Connected {}->{}", m_id,
                    m_connection.second, m_connection.first);
    return true;
  } else {
    ctx.logger.error("[" __FUNCTION__ "][link_id={}, {}->{}]  Not Found!", m_id,
                     c.target.id, c.source.id);
    return false;
  }
}

//
// ShaderGraphToGraphvizCommand class:
//

ShaderGraphToGraphvizCommand::ShaderGraphToGraphvizCommand(
  const ShaderGraph &g, const std::filesystem::path &p)
    : m_graph{&g}, m_path{p} {}

bool ShaderGraphToGraphvizCommand::execute(const Context &ctx) {
  std::ostringstream oss;
  m_graph->exportGraphviz(oss);
  if (!m_path.empty()) {
    os::FileSystem::saveText(m_path, oss.str());
    ctx.logger.info("[" __FUNCTION__ "] -> '{}'", m_path.string());
  } else {
    ImGui::SetClipboardText(oss.str().c_str());
    ctx.logger.info("[" __FUNCTION__ "] -> Clipboard");
  }
  return true;
}
