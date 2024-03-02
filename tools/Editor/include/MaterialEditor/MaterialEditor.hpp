#pragma once

#include "WidgetWindow.hpp"
#include "MaterialProject.hpp"
#include "MaterialEditor/CustomNodeEditor.hpp"
#include "MaterialPreviewWidget.hpp"

#include "sol/state.hpp"
#include "spdlog/logger.h"

class MaterialEditor final : public WidgetWindow {
public:
  MaterialEditor(os::InputSystem &, gfx::WorldRenderer &);
  ~MaterialEditor() override;

  void clear();

  void show(const char *name, bool *open) override;

  void onRender(rhi::CommandBuffer &, float dt) override;

private:
  void _loadFunctions();

  void _initMaterial(gfx::MaterialDomain);

  bool _loadProject(const std::filesystem::path &);

  bool _composeMaterial();
  bool _composeMaterialAndUpdatePreview();
  bool _forceUpdatePreview();

  // ---

  [[nodiscard]] bool _menuBar();
  void _setupDockSpace(const ImGuiID) const;

  void _nodeListWidget(ShaderGraph &);

  [[nodiscard]] bool _canvasWidget(ShaderGraph &,
                                   const std::set<VertexDescriptor> &);

  void _nodePopup(const char *name, ShaderGraph &);
  // @return true if a change has been made in a given set.
  [[nodiscard]] bool _inspectNodes(ShaderGraph &,
                                   const std::set<VertexDescriptor> &);
  void _renderLinks(const ShaderGraph &);
  // @return true if a link has been created between any of vertices of a given
  // set.
  [[nodiscard]] bool _handleNewLinks(ShaderGraph &,
                                     const std::set<VertexDescriptor> &);
  // @return true if any of vertices between any of deleted link is present in a
  // given set.
  [[nodiscard]] bool _handleDeletedLinks(ShaderGraph &,
                                         const std::set<VertexDescriptor> &);
  // @return true if any of deleted nodes is present in a given set.
  [[nodiscard]] bool _handleDeletedNodes(ShaderGraph &,
                                         const std::set<VertexDescriptor> &);

  bool _settingsWidget();

private:
  std::shared_ptr<spdlog::logger> m_logger;

  sol::state m_luaState;
  ScriptedFunctions m_scriptedFunctions;

  // ---

  MaterialProject m_project;
  rhi::ShaderType m_shaderType{rhi::ShaderType::Fragment};

  struct MiniMap {
    bool enabled{true};
    float size{0.15f};
    ImNodesMiniMapLocation location{ImNodesMiniMapLocation_BottomRight};
  };
  MiniMap m_miniMap;

  CustomNodeWidget m_customNodeWidget;

  bool m_livePreview{true};
  MaterialPreviewWidget m_previewWidget;
};
