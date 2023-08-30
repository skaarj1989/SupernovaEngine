#include "MaterialEditor/TextureParam.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"
#include "Services.hpp"

#include "TexturePreview.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "MaterialEditor/ImNodesHelper.hpp"

namespace {

[[nodiscard]] DataType getDataType(const TextureParam &param) {
  using enum DataType;

  if (param.texture && *param.texture) {
    switch (param.texture->getType()) {
      using enum rhi::TextureType;

    case Texture2D:
      return Sampler2D;
    case TextureCube:
      return SamplerCube;
    }
  }
  return Undefined;
}

[[nodiscard]] auto makeTextureNameCandidates(int32_t id,
                                             std::string_view userLabel) {
  std::pair<std::string, std::string> name;
  assert(id >= 0);
  name.second = std::format("t_{}", id);
  if (!userLabel.empty()) {
    name.first =
      std::format("t_{}", ShaderCodeComposer::makeIdentifier(userLabel.data()));
  }
  return name;
}

} // namespace

bool inspectNode(int32_t id, std::optional<const char *> userLabel,
                 TextureParam &p) {
  const auto texturePtr = p.texture.get();

  const auto defaultLabel =
    texturePtr ? toString(texturePtr->getType()) : "Texture";
  const auto cstr = userLabel.value_or(defaultLabel);

  constexpr auto kMinWidth = 128.0f;
  const auto nodeWidth = glm::max(ImGui::CalcTextSize(cstr).x, kMinWidth);

  ImNodes::BeginNodeTitleBar();
  ImGui::TextUnformatted(cstr);
  ImNodes::EndNodeTitleBar();

  if (texturePtr) {
    if (ImGui::IsKeyDown(ImGuiMod_Alt) && ImGui::IsItemHovered()) {
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {8, 8});
      ImGui::SetTooltip("Format: %s", toString(texturePtr->getPixelFormat()));
      ImGui::PopStyleVar();
    }
  }

  auto changed = false;

  const auto aspectRatio =
    texturePtr ? texturePtr->getExtent().getAspectRatio() : 1.0f;
  const auto imageSize = glm::vec2{nodeWidth, nodeWidth * (1.0f / aspectRatio)};

  preview(texturePtr, imageSize);
  if (ImGui::BeginDragDropTarget()) {
    if (auto incomingResource = extractResourceFromPayload(
          kImGuiPayloadTypeTexture, Services::Resources::Textures::value());
        incomingResource) {
      if (auto r = incomingResource->handle(); p.texture != r) {
        p.texture = std::move(r);
        changed = true;
      }
    }
    ImGui::EndDragDropTarget();
  }

  ImGui::Spacing();

  ImNodes::AddOutputAttribute(
    id,
    {
      .name = texturePtr ? toString(getDataType(p)) : "out",
      .color = texturePtr ? std::optional{ImColor{86, 156, 214}} : std::nullopt,
      .nodeWidth = nodeWidth,
    });

  return changed;
}

NodeResult evaluate(MaterialGenerationContext &context, int32_t id,
                    std::optional<const char *> userLabel,
                    const TextureParam &param) {
  if (const auto dataType = getDataType(param);
      dataType != DataType::Undefined) {
    const auto name = emplaceWithFallback(
      context.textures, makeTextureNameCandidates(id, userLabel.value_or("")),
      param.texture);
    if (!name.empty()) {
      return ShaderToken{
        .name = name,
        .dataType = dataType,
      };
    }
  }
  return std::unexpected{"No texture."};
}
