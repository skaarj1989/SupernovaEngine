#pragma once

#include "ShaderGraphStageView.hpp"
#include "NodeUIVisitor.hpp"
#include "CustomNodeEditor.hpp"
#include "NodeContextMenuEntries.hpp"

#include "spdlog/fwd.h"
#include "imnodes.h"

class NodeFactoryRegistry;

class NodeEditor {
public:
  explicit NodeEditor(const NodeFactoryRegistry &);
  ~NodeEditor();

  void showCanvas(MaterialProject &, const ShaderGraphStageViewMutable,
                  spdlog::logger &);
  void showNodeList(const ShaderGraphStageViewConst);

  void refreshFunctionEntries(const ScriptedFunctions &);
  void refreshFunctionEntries(const UserFunctions &);
  void purgeUserFunctionMenu();

private:
  void _displayNodeRegistryMenu(const char *name, ShaderGraphStageViewMutable,
                                const gfx::Material::Surface *);
  void _nodeCanvas(const ShaderGraphStageViewMutable,
                   const gfx::Material::Surface *);
  void _inspectNodes(const ShaderGraphStageViewMutable,
                     const gfx::Material::Surface *);
  void _nodeContextMenu(const char *name, const ShaderGraphStageViewMutable,
                        const NodeBase *);
  void _handleNewLinks(ShaderGraph &);
  void _handleDeletion(const ShaderGraphStageViewMutable);

  void _renderLinks(const ShaderGraph &);

  void _selectAllNodes(const ShaderGraphStageViewMutable);

private:
  MaterialProject *m_currentProject{nullptr};

  const NodeFactoryRegistry *m_nodeFactoryRegistry{nullptr};
  NodeUIVisitor m_nodeUIVisitor;
  MenuEntries m_nodeMenuEntries;
  CustomNodeWidget m_customNodeWidget;

  struct MiniMap {
    bool enabled{true};
    float size{0.15f};
    ImNodesMiniMapLocation location{ImNodesMiniMapLocation_BottomRight};
  };
  MiniMap m_miniMap;

  bool m_focused{false};
};
