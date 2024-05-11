#include "SceneEditor.hpp"
#include "Services.hpp"

#include "ResourceInspector.hpp"
#include "IconsFontAwesome6.h"
#include "ImGuiPopups.hpp" // attachPopup
#include "ImGuiDragAndDrop.hpp"

#include "FileDialog.hpp"

namespace {

[[nodiscard]] auto getScriptTemplateCode() {
  constexpr auto kTemplateScriptPath = "./assets/EntityScriptTemplate.lua";
  return os::FileSystem::readText(kTemplateScriptPath).value_or("");
}
[[nodiscard]] auto createScriptFromTemplate(const std::filesystem::path &p) {
  os::FileSystem::saveText(p, getScriptTemplateCode());
  return Services::Resources::Scripts::value().load(p);
}

} // namespace

void SceneEditor::_onInspect(entt::handle h, const ScriptComponent &c) {
  const auto scriptResource = c.getResource();

  print(scriptResource.get());
  if (ImGui::BeginDragDropTarget()) {
    if (auto incomingResource =
          extractResourceFromPayload(Services::Resources::Scripts::value());
        incomingResource) {
      if (auto r = incomingResource->handle(); scriptResource != r) {
        h.replace<ScriptComponent>(r);
      }
    }
    ImGui::EndDragDropTarget();
  }

  static constexpr auto kCreateScriptModalId = "CreateScript";

  std::optional<const char *> action{};
  attachPopup(IM_UNIQUE_ID, ImGuiMouseButton_Right,
              [this, &h, &c, &action, hasScript = bool(scriptResource)] {
                if (ImGui::MenuItem(ICON_FA_SCROLL " New", nullptr, nullptr,
                                    !hasScript)) {
                  action = kCreateScriptModalId;
                }
                if (ImGui::MenuItem(ICON_FA_FILE_PEN " Edit", nullptr, nullptr,
                                    hasScript)) {
                  publish(EditScriptRequest{c.getResource()});
                }
                if (ImGui::MenuItem(ICON_FA_ERASER " Remove", nullptr, nullptr,
                                    hasScript)) {
                  h.replace<ScriptComponent>();
                }
              });

  if (action) ImGui::OpenPopup(*action);

  constexpr auto kScriptExtension = ".lua";
  constexpr auto scriptFilter = makeExtensionFilter(kScriptExtension);

  static auto dir = os::FileSystem::getRoot();
  if (const auto p =
        showFileDialog(kCreateScriptModalId,
                       {
                         .dir = dir,
                         .barrier = os::FileSystem::getRoot(),
                         .entryFilter = scriptFilter,
                         .forceExtension = kScriptExtension,
                         .flags = FileDialogFlags_AskOverwrite |
                                  FileDialogFlags_CreateDirectoryButton,
                       });
      p) {
    auto resource = createScriptFromTemplate(*p);
    h.replace<ScriptComponent>(resource.handle());
  }
}
