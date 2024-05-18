#include "TextureCache.hpp"
#include "StringUtility.hpp"
#include "renderer/TextureManager.hpp"
#include "TexturePreview.hpp"
#include "ImGuiTitleBarMacro.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "ImGuiModal.hpp"
#include "ImGuiPopups.hpp"

namespace {

void print(const rhi::Texture &texture) {
  assert(texture);

  ImGui::BulletText("Type: %s", toString(texture.getType()));
  ImGui::BulletText("Format: %s", toString(texture.getPixelFormat()));
  const auto extent = texture.getExtent();
  ImGui::BulletText("Dimensions: [%ux%ux%u]", extent.width, extent.height,
                    texture.getDepth());
  ImGui::BulletText("Mip levels: %u", texture.getNumMipLevels());
  ImGui::BulletText("Layers: %u", texture.getNumLayers());
  ImGui::BulletText("Size: %s", formatBytes(texture.getSize()).c_str());
}

void view(gfx::TextureCache &cache, const float windowWidth,
          const float thumbnailSize) {
  if (cache.empty()) {
    ImGui::TextUnformatted("(empty)");
    return;
  }

  std::optional<entt::id_type> junk;
  for (auto [id, resource] : cache) {
    assert(resource);

    ImGui::PushID(id);

    preview(resource.handle().get(), glm::vec2{thumbnailSize});
    onDragSource<rhi::Texture>(
      id, [] { ImGui::TextUnformatted("Texture inside ..."); });

    const auto path = resource->getPath().string();

    if (ImGui::IsKeyDown(ImGuiMod_Alt) && ImGui::IsItemHovered()) {
      ImGui::BeginTooltip();

      ImGui::BulletText("ID: %u", id);
      ImGui::PrintPath(path);

      ImGui::Separator();
      print(*resource);

      ImGui::EndTooltip();
    }

    static constexpr auto kConfirmDestructionId = MAKE_WARNING("Remove");

    std::optional<const char *> action;
    attachPopup(path.c_str(), ImGuiMouseButton_Right, [&action] {
      if (ImGui::MenuItem(" Remove")) {
        action = kConfirmDestructionId;
      }
    });
    if (action) ImGui::OpenPopup(*action);

    if (const auto button =
          showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
            kConfirmDestructionId,
            "Do you really want to remove the resource?");
        button == ModalButton::Yes) {
      junk = id;
    }

    ImGui::PopID();

    const auto lastThumbnailX2 = ImGui::GetItemRectMax().x;
    const auto nextThumbnailX2 =
      lastThumbnailX2 + ImGui::GetStyle().ItemSpacing.x + thumbnailSize;
    if (nextThumbnailX2 < windowWidth) ImGui::SameLine();
  }

  if (junk) cache.erase(*junk);
}

} // namespace

void show(const char *name, bool *open, gfx::TextureManager &cache) {
  if (ImGui::Begin(name, open, ImGuiWindowFlags_MenuBar)) {
    static auto thumbnailSize = 32.0f;
    if (ImGui::BeginMenuBar()) {
      if (ImGui::BeginMenu("Menu")) {
        if (ImGui::BeginMenu("Thumbnail size")) {
          if (ImGui::MenuItem("32x32")) thumbnailSize = 32;
          if (ImGui::MenuItem("64x64")) thumbnailSize = 64;
          if (ImGui::MenuItem("128x128")) thumbnailSize = 128;
          ImGui::EndMenu();
        }
        if (ImGui::MenuItem(ICON_FA_ERASER " Clear")) {
          cache.clear();
        }
        ImGui::EndMenu();
      }
      ImGui::EndMenuBar();
    }

    const auto windowVisibleX2 =
      ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    view(cache, windowVisibleX2, thumbnailSize);
  }
  ImGui::End();
}
