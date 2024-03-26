#pragma once

#include "WidgetWindow.hpp"
#include "MaterialProject.hpp"
#include "NodeFactoryRegistry.hpp"
#include "NodeEditor.hpp"
#include "MaterialPreviewWidget.hpp"

#include "sol/state.hpp"
#include "spdlog/logger.h"

class MaterialEditor final : public WidgetWindow {
public:
  MaterialEditor(os::InputSystem &, gfx::WorldRenderer &);
  ~MaterialEditor() override;

  void clear();

  void show(const char *name, bool *open) override;
  void onRender(rhi::CommandBuffer &, const float dt) override;

private:
  void _loadFunctions();
  void _connectProject();

  void _setupDockSpace(const ImGuiID) const;

  void _settingsWidget();

private:
  std::shared_ptr<spdlog::logger> m_logger;
  sol::state m_luaState;

  // ---

  MaterialProject m_project;
  ScriptedFunctions m_scriptedFunctions;
  NodeFactoryRegistry m_nodeFactoryRegistry;
  NodeEditor m_nodeEditor;
  MaterialPreviewWidget m_previewWidget;

  rhi::ShaderType m_shaderType{rhi::ShaderType::Fragment};
  bool m_livePreview{true};

  struct Windows {
    bool canvas{true};
    bool nodeList{true};
    bool codeEditor{true};

    bool settings{true};
    bool sceneSettings{true};
    bool renderSettings{true};

    bool preview{true};
  };
  Windows m_windows;
};
