#include "NodesInternal.hpp"

#include "TypeTraits.hpp"
#include "AlwaysFalse.hpp"
#include "StringUtility.hpp"

#include "IconsFontAwesome6.h"

#include <array>
#include <format>

namespace {

[[nodiscard]] bool inspect(VertexProp::Variant &variant,
                           InspectorMode inspectorMode, bool allowTypeChange) {
  return std::visit(
    [inspectorMode, allowTypeChange](auto &&arg) {
      using T = std::decay_t<decltype(arg)>;

      if constexpr (std::is_empty_v<T>) {
        return false;
      } else if constexpr (is_any_v<T, MasterNodeVariant, CompoundNodeVariant,
                                    SplitVector, SplitMatrix>) {
        assert(false);
        return false;
      } else if constexpr (std::is_same_v<T, ContainerNode>) {
        return false;
      } else if constexpr (std::is_same_v<T, ValueVariant>) {
        const auto inspect = [&] {
          ImGui::PushItemWidth(calcOptimalInspectorWidth(getDataType(arg)));
          auto changed = ::inspect(arg);
          if (allowTypeChange) {
            ImGui::SameLine();
            changed |= changeValueCombo(IM_UNIQUE_ID, arg);
          }
          ImGui::PopItemWidth();
          return changed;
        };

        auto changed = false;
        switch (inspectorMode) {
        case InspectorMode::Inline:
          changed |= inspect();
          break;
        case InspectorMode::Popup: {
          constexpr auto kPopupId = IM_UNIQUE_ID;
          if (ImGui::SmallButton(ICON_FA_PENCIL)) ImGui::OpenPopup(kPopupId);

          ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{4.0f, 4.0f});
          if (ImGui::BeginPopup(kPopupId, ImGuiWindowFlags_NoMove)) {
            changed |= inspect();
            ImGui::EndPopup();
          }
          ImGui::PopStyleVar();
        } break;
        }
        return changed;
      } else if constexpr (is_any_v<T, PropertyValue, TextureParam>) {
        return false;
      } else if constexpr (is_any_v<T, Attribute, FrameBlockMember,
                                    CameraBlockMember, BuiltInConstant,
                                    BuiltInSampler>) {
        static const ImColor kTypeColor{184, 215, 163};
        ImGui::TextColored(kTypeColor, "[%s]", ::toString(arg));
        return false;
      } else {
        static_assert(always_false_v<T>, "non-exhaustive visitor!");
      }
    },
    variant);
}

template <typename T>
constexpr auto is_simple_variant_v =
  is_any_v<T, std::monostate, ValueVariant, Attribute, PropertyValue,
           FrameBlockMember, CameraBlockMember, BuiltInConstant, BuiltInSampler,
           SplitVector, SplitMatrix, TextureParam>;

[[nodiscard]] auto canCopy(const VertexProp::Variant &v) {
  return std::visit(
    [](auto &&arg) {
      using T = std::decay_t<decltype(arg)>;

      if constexpr (is_simple_variant_v<T> ||
                    std::is_same_v<T, ContainerNode>) {
        return true;
      } else if constexpr (std::is_same_v<T, CompoundNodeVariant>) {
        return false;
      } else if constexpr (std::is_same_v<T, MasterNodeVariant>) {
        assert(false);
        return false;
      } else {
        static_assert(always_false_v<T>, "non-exhaustive visitor!");
      }
    },
    v);
}

} // namespace

std::string buildFunctionCall(const std::string_view functionName,
                              const std::vector<std::string> &args) {
  return std::format("{}({})", functionName, join(args, ", "));
}

bool addInputPin(ShaderGraph &g, VertexDescriptor vd,
                 const ImNodes::InputAttributeParams &params,
                 InspectorMode inspectorMode, bool allowTypeChange) {
  auto changed = false;

  auto &vertexProp = g.getVertexProp(vd);

  params.active ? ImNodes::BeginInputAttribute(vertexProp.id)
                : ImNodes::BeginStaticAttribute(vertexProp.id);

  ImGui::BeginDisabled(!params.active);
  ImGui::TextUnformatted(params.name.data());
  if (params.active && g.countOutEdges(vd) == 0) {
    ImGui::SameLine();
    changed |= inspect(vertexProp.variant, inspectorMode, allowTypeChange);
  }
  ImGui::EndDisabled();
  params.active ? ImNodes::EndInputAttribute() : ImNodes::EndStaticAttribute();

  ImGui::Spacing();

  return changed;
}

void addOutputPins(ShaderGraph &g, const VectorSplitterNode::Output &out,
                   SwizzleMaskSet swizzleMaskSet, float nodeWidth) {
  constexpr auto kSwizzleMaskSets = std::array{
    std::array{"xyz", "x", "y", "z", "w"},
    std::array{"rgb", "r", "g", "b", "a"},
    std::array{"stp", "s", "t", "p", "q"},
  };

  struct Item {
    VertexDescriptor vd{nullptr};
    int32_t index{0};
    std::optional<ImU32> color;
  };
  std::initializer_list<Item> pins{
    {out.xyz, 0, std::nullopt},
    {out.x, 1, ImColor{1.0f, 0.2f, 0.321f}},
    {out.y, 2, ImColor{0.545f, 0.862f, 0.0f}},
    {out.z, 3, ImColor{0.156f, 0.563f, 1.0f}},
    {out.w, 4, IM_COL32(255, 255, 255, 127)},
  };

  for (const auto &[vd, idx, color] : pins) {
    const auto &vertexProp = g.getVertexProp(vd);
    ImNodes::AddOutputAttribute(
      vertexProp.id,
      {
        .name = kSwizzleMaskSets[std::to_underlying(swizzleMaskSet)][idx],
        .color = color,
        .nodeWidth = nodeWidth,
      });
  }
}

void copySimpleVariant(ShaderGraph &g, VertexDescriptor sourceVd,
                       VertexDescriptor targetVd) {
  const auto &sourceProp = g.getVertexProp(sourceVd);
  if (canCopy(sourceProp.variant)) {
    g.getVertexProp(targetVd).variant = sourceProp.variant;
  } else {
    assert(false);
  }
}
