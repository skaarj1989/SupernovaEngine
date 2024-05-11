#pragma once

#include "Command.hpp"
#include "ShaderGraphCommon.hpp"
#include "ShaderGraphStageView.hpp"
#include "glm/vec2.hpp"

#include <filesystem>

class NodeFactoryRegistry;
using FactoryID = std::size_t;

struct ImNodesEditorContext;

class MutateGraphCommand : public UndoableCommand {
public:
  explicit MutateGraphCommand(const ShaderGraphStageViewMutable view)
      : m_view{view} {}

protected:
  ShaderGraphStageViewMutable m_view;
};

class CreateNodeCommand : public MutateGraphCommand {
public:
  CreateNodeCommand(const ShaderGraphStageViewMutable,
                    const NodeFactoryRegistry &, const FactoryID,
                    const glm::vec2 position);

  bool execute(const Context &) override;
  bool undo(const Context &) override;

private:
  const NodeFactoryRegistry *m_registry;
  FactoryID m_factoryId;
  glm::vec2 m_position;
  std::optional<VertexID> m_id{}; // undo
};
class RemoveNodeCommand : public MutateGraphCommand {
public:
  RemoveNodeCommand(const ShaderGraphStageViewMutable, const VertexID);

  bool execute(const Context &) override;
  bool undo(const Context &) override;

private:
  VertexID m_id;
  // -- Undo:
  std::string m_memento;
  glm::vec2 m_position{};
};
class CloneNodeCommand : public MutateGraphCommand {
public:
  CloneNodeCommand(const ShaderGraphStageViewMutable, const VertexID,
                   const glm::vec2 position);

  bool execute(const Context &) override;
  bool undo(const Context &) override;

private:
  VertexID m_sourceId;
  glm::vec2 m_position;
  std::optional<VertexID> m_id{}; // undo
};
class RenameNodeCommand : public MutateGraphCommand {
public:
  RenameNodeCommand(ShaderGraph &, const VertexID, const std::string &);

  bool execute(const Context &) override;
  bool undo(const Context &) override;

private:
  VertexID m_id;
  std::string m_name; // undo
};

class CreateLinkCommand : public MutateGraphCommand {
public:
  CreateLinkCommand(ShaderGraph &, const ConnectedIDs);

  bool execute(const Context &) override;
  bool undo(const Context &) override;

private:
  ConnectedIDs m_connection;
  std::optional<EdgeID> m_id{}; // undo
};
class RemoveLinkCommand : public MutateGraphCommand {
public:
  RemoveLinkCommand(ShaderGraph &, const EdgeID);

  bool execute(const Context &) override;
  bool undo(const Context &) override;

private:
  EdgeID m_id;
  ConnectedIDs m_connection{}; // undo
};

class ShaderGraphToGraphvizCommand : public Command {
public:
  ShaderGraphToGraphvizCommand(const ShaderGraph &,
                               const std::filesystem::path & = {});

  bool execute(const Context &) override;

private:
  const ShaderGraph *m_graph;
  std::filesystem::path m_path;
};
