#include "Inspectors/PostProcessEffectInspector.hpp"
#include "Inspectors/MaterialInstanceInspector.hpp"
#include "Inspectors/ResourceInspector.hpp"

#include "Services.hpp"

#include "ImGuiTitleBarMacro.hpp"
#include "ImGuiModal.hpp"
#include "ImGuiDragAndDrop.hpp"

#include <ranges>

namespace {

struct Reorder {
  std::size_t from;
  std::size_t to;
};
struct Remove {
  std::size_t index;
};
using Request = std::variant<Reorder, Remove>;

void handle(std::vector<gfx::MaterialInstance> &effects, const Reorder &req) {
  std::iter_swap(effects.begin() + req.from, effects.begin() + req.to);
}
void handle(std::vector<gfx::MaterialInstance> &effects, const Remove &req) {
  effects.erase(effects.cbegin() + req.index);
}

} // namespace

void inspectPostProcessEffects(
  std::vector<gfx::MaterialInstance> &postProcessEffects) {
  if (ImGui::Button(ICON_FA_FILM " Add Effect"))
    postProcessEffects.emplace_back();

  constexpr auto kUseDragDrop = true;

  std::optional<Request> request;

  constexpr auto kRemoveEffectModalId = MAKE_WARNING("Remove effect");

  for (auto [i, effect] : std::views::enumerate(postProcessEffects)) {
    ImGui::PushID(i);

    const auto name = effect ? effect->getName() : "";
    const auto label = !name.empty() ? std::string{name} : "(no name)";

    auto visible = true;
    auto treeOpened = ImGui::CollapsingHeader(
      label.c_str(), &visible,
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick);

    if (!visible) ImGui::OpenPopup(kRemoveEffectModalId);

    if (const auto button =
          showMessageBox<ModalButtons::Yes | ModalButtons::Cancel>(
            kRemoveEffectModalId,
            std::format(
              "Remove the following effect? (this cannot be undone)\n- {}",
              label)
              .c_str());
        button && *button == ModalButton::Yes) {
      request = Remove{.index = std::size_t(i)};
      treeOpened = false;
    }

    if constexpr (kUseDragDrop) {
      constexpr auto kDragDropType = "POSTPROCESS";
      if (ImGui::BeginDragDropSource(
            ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) {
        ImGui::SetDragDropPayload(kDragDropType, &i, sizeof(std::size_t));
        ImGui::SetTooltip("Effect: %s", label.c_str());
        ImGui::EndDragDropSource();
      }
      if (ImGui::BeginDragDropTarget()) {
        if (const auto *payload = ImGui::AcceptDragDropPayload(
              kDragDropType, ImGuiDragDropFlags_SourceNoHoldToOpenOthers)) {
          request = Reorder{
            .from = *static_cast<const std::size_t *>(payload->Data),
            .to = std::size_t(i),
          };
        }
        ImGui::EndDragDropTarget();
      }
    } else {
      // FIXME: Jitter (with elements of the same name).
      if (ImGui::IsItemActive() && !ImGui::IsItemHovered()) {
        const auto next =
          i +
          (ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y < 0.0f ? -1 : 1);
        if (next >= 0 && next < postProcessEffects.size()) {
          request = Reorder{.from = std::size_t(i), .to = std::size_t(next)};
          ImGui::ResetMouseDragDelta();
        }
      }
    }

    if (treeOpened) {
      const auto materialResource =
        std::dynamic_pointer_cast<Resource>(effect.getPrototype()).get();

      print(materialResource);
      if (ImGui::BeginDragDropTarget()) {
        if (auto incomingResource = extractResourceFromPayload(
              kImGuiPayloadTypeMaterial,
              Services::Resources::Materials::value());
            incomingResource) {
          if (auto r = incomingResource->handle();
              effect.getPrototype() != r && !isSurface(*r)) {
            effect = gfx::MaterialInstance{std::move(r)};
          }
        }
        ImGui::EndDragDropTarget();
      }
      inspect(effect);
    }
    ImGui::PopID();

    ++i;
  }

  if (request) {
    std::visit([&postProcessEffects](
                 const auto &arg) { handle(postProcessEffects, arg); },
               *request);
  }
}
