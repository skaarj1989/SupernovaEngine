#include "ResourceDockSpace.hpp"
#include "Services.hpp"
#include "ImGuiHelper.hpp"
#include "imgui_internal.h" // DockSpace
#include "MeshCache.hpp"
#include "MaterialCache.hpp"
#include "TextureCache.hpp"
#include "SkeletonCache.hpp"
#include "AnimationCache.hpp"
#include "ColliderCache.hpp"
#include "AudioClipCache.hpp"
#include "ScriptCache.hpp"

ResourcesWidget::ResourcesWidget() {
#define ADD_CACHE(Name)                                                        \
  m_widgets.add<SimpleWidgetWindow>(                                           \
    #Name, {.name = #Name, .open = true}, [](const char *name, bool *open) {   \
      ::show(name, open, Services::Resources::Name::value());                  \
    })

  ADD_CACHE(Meshes);
  ADD_CACHE(Materials);
  ADD_CACHE(Textures);

  ADD_CACHE(Skeletons);
  ADD_CACHE(Animations);

  ADD_CACHE(Colliders);

  ADD_CACHE(AudioClips);

  ADD_CACHE(Scripts);

#undef ADD_CACHE
}

void ResourcesWidget::show(const char *name, bool *open) {
  ZoneScopedN("ResourcesWidget");
  ImGui::Begin(name, open, ImGuiWindowFlags_MenuBar);
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("Cache")) {
      ImGui::MenuItems(m_widgets);
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  const auto dockspaceId = ImGui::GetID("DockSpace");
  _setupDockSpace(dockspaceId);
  ImGui::DockSpace(dockspaceId);
  ImGui::End();

  ::show(m_widgets);
}

void ResourcesWidget::_setupDockSpace(const ImGuiID dockspaceId) const {
  if (static auto firstTime = true; firstTime) [[unlikely]] {
    firstTime = false;

    if (ImGui::DockBuilderGetNode(dockspaceId) == nullptr) {
      ImGui::DockBuilderRemoveNode(dockspaceId);
      ImGui::DockBuilderAddNode(dockspaceId);
      for (const auto &[_, entry] : m_widgets) {
        ImGui::DockBuilderDockWindow(entry.config.name, dockspaceId);
      }
      ImGui::DockBuilderFinish(dockspaceId);
    }
  }
}
