#include "Inspectors/MaterialInstanceInspector.hpp"
#include "Inspectors/PropertyInspector.hpp"

#include "Services.hpp"

#include "TexturePreview.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "ImGuiHelper.hpp"

namespace {

void inspectTextures(gfx::MaterialInstance &materialInstance) {
  constexpr auto kTableFlags = ImGuiTableFlags_Borders |
                               ImGuiTableFlags_SizingFixedFit |
                               ImGuiTableFlags_NoHostExtendX;
  if (ImGui::BeginTable(IM_UNIQUE_ID, 3, kTableFlags)) {
    ImGui::TableSetupColumn("slot");
    ImGui::TableSetupColumn("type");
    ImGui::TableSetupColumn("preview", ImGuiTableColumnFlags_NoHeaderLabel);
    ImGui::TableHeadersRow();

    constexpr auto kPreviewSize = 48.0f;
    constexpr auto kSelectableSize = ImVec2{0, kPreviewSize};
    for (auto &[name, textureInfo] : materialInstance.getTextures()) {
      ImGui::TableNextRow();

      ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, {0.0f, 0.5f});

      ImGui::TableSetColumnIndex(0);
      ImGui::Selectable(name.c_str(), false,
                        ImGuiSelectableFlags_SpanAllColumns, kSelectableSize);

      if (ImGui::BeginDragDropTarget()) {
        if (auto incomingResource = extractResourceFromPayload(
              kImGuiPayloadTypeTexture, Services::Resources::Textures::value());
            incomingResource) {
          materialInstance.setTexture(name, incomingResource->handle());
        }
        ImGui::EndDragDropTarget();
      }

      ImGui::TableSetColumnIndex(1);
      ImGui::Selectable(toString(textureInfo.type), false,
                        ImGuiSelectableFlags_Disabled, kSelectableSize);

      ImGui::PopStyleVar();

      ImGui::TableSetColumnIndex(2);
      preview(textureInfo.texture.get(), glm::vec2{kPreviewSize});
    }
    ImGui::EndTable();
  }
}

} // namespace

void inspect(gfx::MaterialInstance &materialInstance) {
  if (const auto *prototype = materialInstance.getPrototype().get();
      prototype) {
    ImGui::SeparatorText("textures");
    if (materialInstance.hasTextures()) {
      inspectTextures(materialInstance);
    } else {
      ImGui::BulletText("(none)");
    }
    ImGui::SeparatorText("properties");
    if (materialInstance.hasProperties()) {
      inspect(materialInstance.getProperties());
    } else {
      ImGui::BulletText("(none)");
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    auto dirty = false;
    auto flags = materialInstance.getFlags();
    if (gfx::isSurface(*prototype)) {
      dirty |= ImGui::CheckboxFlags("CastShadow", flags,
                                    gfx::MaterialFlags::CastShadow);
      ImGui::SameLine();
      dirty |= ImGui::CheckboxFlags("ReceiveShadow", flags,
                                    gfx::MaterialFlags::ReceiveShadow);
    } else {
      dirty |=
        ImGui::CheckboxFlags("Enabled", flags, gfx::MaterialFlags::Enabled);
    }
    if (dirty) materialInstance.setFlags(flags);
  }
}
