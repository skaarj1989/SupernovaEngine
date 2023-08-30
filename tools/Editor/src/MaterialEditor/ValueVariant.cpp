#include "MaterialEditor/ValueVariant.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"

#include "VisitorHelper.hpp"

#if __has_include("imnodes.h")
#  include "TypeTraits.hpp"
#  include "AlwaysFalse.hpp"

#  include "MaterialEditor/ChangeVariantCombo.hpp"
#  include "MaterialEditor/ImNodesHelper.hpp"

#  include "ImGuiHelper.hpp"

#  include "glm/gtc/type_ptr.hpp" // value_ptr

#  include <array>
#  include <format>
#endif

DataType getDataType(const ValueVariant &v) {
  using enum DataType;

  return std::visit(Overload{
                      [](bool) { return Bool; },
                      [](glm::bvec2) { return BVec2; },
                      [](glm::bvec3) { return BVec3; },
                      [](glm::bvec4) { return BVec4; },

                      [](int32_t) { return Int32; },
                      [](glm::ivec2) { return IVec2; },
                      [](glm::ivec3) { return IVec3; },
                      [](glm::ivec4) { return IVec4; },

                      [](uint32_t) { return UInt32; },
                      [](glm::uvec2) { return UVec2; },
                      [](glm::uvec3) { return UVec3; },
                      [](glm::uvec4) { return UVec4; },

                      [](float) { return Float; },
                      [](glm::vec2) { return Vec2; },
                      [](glm::vec3) { return Vec3; },
                      [](glm::vec4) { return Vec4; },

                      [](double) { return Double; },
                      [](glm::dvec2) { return DVec2; },
                      [](glm::dvec3) { return DVec3; },
                      [](const glm::dvec4 &) { return DVec4; },

                      [](const glm::mat2 &) { return Mat2; },
                      [](const glm::mat3 &) { return Mat3; },
                      [](const glm::mat4 &) { return Mat4; },
                    },
                    v);
}

const char *toString(const ValueVariant &v) { return toString(getDataType(v)); }

#if __has_include("imnodes.h")
namespace {

template <typename T> constexpr ImGuiDataType getImGuiDataType() {
  if constexpr (std::is_same_v<T, int8_t>) {
    return ImGuiDataType_S8;
  } else if constexpr (std::is_same_v<T, uint8_t>) {
    return ImGuiDataType_U8;
  } else if constexpr (std::is_same_v<T, int16_t>) {
    return ImGuiDataType_S16;
  } else if constexpr (std::is_same_v<T, uint16_t>) {
    return ImGuiDataType_U16;
  } else if constexpr (std::is_same_v<T, int32_t>) {
    return ImGuiDataType_S32;
  } else if constexpr (std::is_same_v<T, uint32_t>) {
    return ImGuiDataType_U32;
  } else if constexpr (std::is_same_v<T, float>) {
    return ImGuiDataType_Float;
  } else if constexpr (std::is_same_v<T, double>) {
    return ImGuiDataType_Double;
  }

  assert(false);
  return ImGuiDataType_COUNT;
}

} // namespace

bool inspectNode(int32_t id, std::optional<const char *> userLabel,
                 ValueVariant &v) {
  ImNodes::BeginNodeTitleBar();
  ImGui::BeginGroup();
  auto changed = changeValueCombo(IM_UNIQUE_ID, v);
  ImGui::SameLine();
  const auto cstr = userLabel.value_or("Constant");
  ImGui::TextUnformatted(cstr);
  ImGui::EndGroup();
  const auto nodeWidth = ImGui::GetItemRectSize().x;
  ImNodes::EndNodeTitleBar();

  // ---

  ImGui::PushItemWidth(calcOptimalInspectorWidth(getDataType(v)));
  changed |= inspect(v);
  ImGui::PopItemWidth();

  const auto lastItemWidth = ImGui::GetItemRectSize().x;

  ImNodes::AddOutputAttribute(
    id, {.name = toString(getDataType(v)),
         .nodeWidth = glm::max(nodeWidth, lastItemWidth)});

  return changed;
}

bool inspect(ValueVariant &v) {
  ImGui::BeginGroup();

  const auto changed = std::visit(
    [](auto &arg) {
      using T = std::decay_t<decltype(arg)>;

      if constexpr (std::is_same_v<T, bool>) {
        return ImGui::Checkbox(IM_UNIQUE_ID, &arg);
      } else if constexpr (is_any_v<T, glm::bvec2, glm::bvec3, glm::bvec4>) {
        return ImGui::CheckboxN(glm::value_ptr(arg), arg.length());
      } else if constexpr (is_any_v<T, int32_t, uint32_t, float, double>) {
        return ImGui::DragScalarN(IM_UNIQUE_ID, getImGuiDataType<T>(), &arg, 1,
                                  0.1f);
      } else if constexpr (is_any_v<T,
                                    // clang-format off
                                    glm::ivec2, glm::ivec3, glm::ivec4,
                                    glm::uvec2, glm::uvec3, glm::uvec4,
                                    glm::vec2,
                                    glm::dvec2, glm::dvec3, glm::dvec4
                                    // clang-format on
                                    >) {
        return ImGui::DragScalarN(IM_UNIQUE_ID,
                                  getImGuiDataType<typename T::value_type>(),
                                  glm::value_ptr(arg), arg.length(), 0.1f);
      } else if constexpr (std::is_same_v<T, glm::vec3>) {
        return ImGui::ColorEdit3(IM_UNIQUE_ID, glm::value_ptr(arg),
                                 ImGuiColorEditFlags_Float |
                                   ImGuiColorEditFlags_HDR);
      } else if constexpr (std::is_same_v<T, glm::vec4>) {
        return ImGui::ColorEdit4(IM_UNIQUE_ID, glm::value_ptr(arg),
                                 ImGuiColorEditFlags_Float |
                                   ImGuiColorEditFlags_HDR);
      } else if constexpr (is_any_v<T, glm::mat2, glm::mat3, glm::mat4>) {
        auto dirty = false;
        for (auto i = 0; i < arg.length(); ++i) {
          ImGui::PushID(i);
          auto &column = arg[i];
          dirty |=
            ImGui::DragScalarN(IM_UNIQUE_ID, ImGuiDataType_Float,
                               glm::value_ptr(column), column.length(), 0.1f);
          ImGui::PopID();
        }
        return dirty;
      } else {
        static_assert(always_false_v<T>, "non-exhaustive visitor!");
      }
    },
    v);

  ImGui::EndGroup();

  return changed;
}

bool changeValueCombo(const char *label, ValueVariant &v) {
  using Item = Option<ValueVariant>::value_type;
  constexpr auto kNumOptions = 5 * 4 + 3;
  constexpr auto kNumSeparators = 5;
  constexpr auto kOptions =
    std::array<Option<ValueVariant>, kNumOptions + kNumSeparators>{
      Item{"bool", false},
      Item{"bvec2", glm::bvec2{false}},
      Item{"bvec3", glm::bvec3{false}},
      Item{"bvec4", glm::bvec4{false}},

      std::nullopt,

      Item{"int32", 0},
      Item{"ivec2", glm::ivec2{0}},
      Item{"ivec3", glm::ivec3{0}},
      Item{"ivec4", glm::ivec4{0}},

      std::nullopt,

      Item{"uint32", 0u},
      Item{"uvec2", glm::uvec2{0u}},
      Item{"uvec3", glm::uvec3{0u}},
      Item{"uvec4", glm::uvec4{0u}},

      std::nullopt,

      Item{"float", 0.0f},
      Item{"vec2", glm::vec2{0.0f}},
      Item{"vec3", glm::vec3{0.0f}},
      Item{"vec4", glm::vec4{0.0f}},

      std::nullopt,

      Item{"double", 0.0},
      Item{"dvec2", glm::dvec2{0.0}},
      Item{"dvec3", glm::dvec3{0.0}},
      Item{"dvec4", glm::dvec4{0.0}},

      std::nullopt,

      Item{"mat2", glm::mat2{1.0}},
      Item{"mat3", glm::mat3{1.0}},
      Item{"mat4", glm::mat4{1.0}},
    };
  static_assert(std::variant_size_v<ValueVariant> == kNumOptions);

  return changeVariantCombo<ValueVariant>(label, v, kOptions);
}
#endif

NodeResult evaluate(MaterialGenerationContext &context, int32_t id,
                    const ValueVariant &value) {
  ShaderToken token{
    .name = nodeIdToString(id),
    .dataType = getDataType(value),
  };
  context.currentShader->composer.addVariable(token.name, value);
  return token;
}
