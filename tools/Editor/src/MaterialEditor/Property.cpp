#include "MaterialEditor/Property.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"

#include "VisitorHelper.hpp"

#include "MaterialEditor/ChangeVariantCombo.hpp"
#include "MaterialEditor/ImNodesHelper.hpp"
#include "Inspectors/PropertyInspector.hpp"

#include <array>
#include <format>

namespace {

[[nodiscard]] auto changePropertyCombo(const char *label, PropertyValue &v) {
  using Item = Option<PropertyValue>::value_type;
  constexpr auto kNumOptions = std::variant_size_v<PropertyValue>;
  constexpr auto kNumSeparators = 1;
  constexpr auto kOptions =
    std::array<Option<PropertyValue>, kNumOptions + kNumSeparators>{
      Item{"int32", 0},
      Item{"uint32", 0u},

      std::nullopt,

      Item{"float", 0.0f},
      Item{"vec2", glm::vec2{0.0f}},
      Item{"vec4", glm::vec4{0.0f}},
    };
  return changeVariantCombo<PropertyValue>(label, v, kOptions);
}

[[nodiscard]] const char *getPrefix(const rhi::ShaderType shaderType) {
  switch (shaderType) {
    using enum rhi::ShaderType;

  case Vertex:
    return "vs";
  case Fragment:
    return "fs";
  }

  assert(false);
  return "unknown";
}

[[nodiscard]] auto makePropertyName(const rhi::ShaderType shaderType,
                                    int32_t id) {
  assert(id >= 0);
  return std::format("{}_id_{}", getPrefix(shaderType), id);
}
[[nodiscard]] std::string makePropertyName(const rhi::ShaderType shaderType,
                                           std::string_view userLabel) {
  assert(!userLabel.empty());
  return std::format("{}_{}", getPrefix(shaderType), userLabel);
}
[[nodiscard]] auto makePropertyNameCandidates(const rhi::ShaderType shaderType,
                                              int32_t id,
                                              std::string_view userLabel) {
  std::pair<std::string, std::string> name;
  name.second = makePropertyName(shaderType, id);
  if (!userLabel.empty()) {
    name.first = makePropertyName(
      shaderType, ShaderCodeComposer::makeIdentifier(userLabel.data()));
  }
  return name;
}

} // namespace

DataType getDataType(const PropertyValue &v) {
  using enum DataType;
  return std::visit(Overload{
                      [](int32_t) { return Int32; },
                      [](uint32_t) { return UInt32; },
                      [](float) { return Float; },
                      [](glm::vec2) { return Vec2; },
                      [](const glm::vec4 &) { return Vec4; },
                    },
                    v);
}

bool inspectNode(int32_t id, std::optional<const char *> userLabel,
                 PropertyValue &v) {
  ImNodes::BeginNodeTitleBar();
  ImGui::BeginGroup();
  auto changed = changePropertyCombo(IM_UNIQUE_ID, v);
  ImGui::SameLine();

  const auto cstr = userLabel.value_or("Property");
  ImGui::TextUnformatted(cstr);
  ImGui::EndGroup();
  const auto nodeWidth = ImGui::GetItemRectSize().x;
  ImNodes::EndNodeTitleBar();

  ImGui::PushItemWidth(calcOptimalInspectorWidth(getDataType(v)));
  changed |= inspect(IM_UNIQUE_ID, v);
  ImGui::PopItemWidth();

  const auto lastItemWidth = ImGui::GetItemRectSize().x;

  ImNodes::AddOutputAttribute(
    id, {.name = "out", .nodeWidth = glm::max(nodeWidth, lastItemWidth)});

  return changed;
}

NodeResult evaluate(MaterialGenerationContext &context, int32_t id,
                    std::optional<const char *> userLabel,
                    const PropertyValue &v) {
  const auto name =
    emplaceWithFallback(context.properties,
                        makePropertyNameCandidates(context.currentShader->type,
                                                   id, userLabel.value_or("")),
                        v);
  return ShaderToken{
    .name = std::format("properties.{}", name),
    .dataType = getDataType(v),
  };
}
