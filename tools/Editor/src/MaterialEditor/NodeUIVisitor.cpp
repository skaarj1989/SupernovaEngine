#include "MaterialEditor/NodeUIVisitor.hpp"
#include "TypeTraits.hpp"
#include "AlwaysFalse.hpp"
#include "Services.hpp"

#include "MaterialEditor/ValueVariant.hpp"
#include "MaterialEditor/PropertyVariant.hpp"
#include "MaterialEditor/TextureParam.hpp"
#include "MaterialEditor/Attribute.hpp"
#include "MaterialEditor/BuiltInConstants.hpp"
#include "MaterialEditor/FrameBlockMember.hpp"
#include "MaterialEditor/CameraBlockMember.hpp"
#include "MaterialEditor/Nodes/Embedded.hpp"

#include "MaterialEditor/Nodes/Append.hpp"
#include "MaterialEditor/Nodes/VectorSplitter.hpp"
#include "MaterialEditor/Nodes/MatrixSplitter.hpp"
#include "MaterialEditor/Nodes/Swizzle.hpp"
#include "MaterialEditor/Nodes/Arithmetic.hpp"
#include "MaterialEditor/Nodes/MatrixTransform.hpp"
#include "MaterialEditor/Nodes/TextureSampling.hpp"
#include "MaterialEditor/Nodes/Scripted.hpp"
#include "MaterialEditor/Nodes/Custom.hpp"

#include "MaterialEditor/Nodes/VertexMaster.hpp"
#include "MaterialEditor/Nodes/SurfaceMaster.hpp"
#include "MaterialEditor/Nodes/PostProcessMaster.hpp"

#include "MaterialEditor/ShaderGraph.hpp"

#include "MaterialEditor/ChangeEnumCombo.hpp"
#include "MaterialEditor/ChangeVariantCombo.hpp"
#include "Inspectors/PropertyInspector.hpp"
#include "TexturePreview.hpp"

#include "IconsFontAwesome6.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"  // InputTextWithHint
#include "ImGuiHelper.hpp" // CheckboxN
#include "ImGuiDragAndDrop.hpp"
#include "ImNodesHelper.hpp"

#include "glm/gtc/type_ptr.hpp" // value_ptr

#include "tracy/Tracy.hpp"

#include <ranges>

namespace ImGui {

[[nodiscard]] static auto TreeNode2(const char *label, const float width) {
  ImGuiForceItemWidth _{width};
  PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.0f);
  const auto expanded = TreeNodeEx(label, ImGuiTreeNodeFlags_Bullet |
                                            ImGuiTreeNodeFlags_DefaultOpen);
  PopStyleVar();
  return expanded;
}
static void Separator2(const float width) {
  ImGuiForceItemWidth _{width};
  Spacing();
  Separator();
  Spacing();
}

} // namespace ImGui

namespace {

constexpr auto kDefaultPinShape = ImNodesPinShape_Circle;
constexpr auto kTexturePreviewWidth = 128.0f;

constexpr auto kDefaultNodeWidth = 40.0f;

auto defaultTitleBar(const std::string &title,
                     const float minNodeWidth = kDefaultNodeWidth) {
  ImNodes::BeginNodeTitleBar();
  ImGui::TextUnformatted(!title.empty() ? title.c_str() : "(unknown)");
  const auto width = ImGui::GetItemRectSize().x;
  ImNodes::EndNodeTitleBar();
  return glm::max(width, minNodeWidth);
}
auto defaultTitleBar(const NodeBase &node,
                     const float minNodeWidth = kDefaultNodeWidth) {
  return defaultTitleBar(!node.label.empty() ? node.label : node.toString(),
                         minNodeWidth);
}

void updateNodeWidth(float &width) {
  width = glm::max(width, ImGui::GetItemRectSize().x);
}

bool changeValueCombo(const char *label, ValueVariant &value) {
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

  return changeVariantCombo<ValueVariant>(label, value, kOptions);
}
auto changePropertyCombo(const char *label, PropertyVariant &value) {
  using Item = Option<PropertyVariant>::value_type;
  constexpr auto kNumOptions = std::variant_size_v<PropertyVariant>;
  constexpr auto kNumSeparators = 1;
  constexpr auto kOptions =
    std::array<Option<PropertyVariant>, kNumOptions + kNumSeparators>{
      Item{"int32", 0},
      Item{"uint32", 0u},

      std::nullopt,

      Item{"float", 0.0f},
      Item{"vec2", glm::vec2{0.0f}},
      Item{"vec4", glm::vec4{0.0f}},
    };
  return changeVariantCombo<PropertyVariant>(label, value, kOptions);
}

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
  } else {
    static_assert(always_false_v<T>);
  }
}

bool inspect(ValueVariant &value) {
  ImGui::BeginGroup();
  auto changed = changeValueCombo(IM_UNIQUE_ID, value);
  ImGui::SameLine();
  ImGui::PushItemWidth(calcOptimalInspectorWidth(getDataType(value)));
  ImGui::BeginGroup();
  changed |= std::visit(
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
    value);
  ImGui::EndGroup();
  ImGui::PopItemWidth();
  ImGui::EndGroup();
  return changed;
}
bool inspect(PropertyVariant &value) {
  ImGui::BeginGroup();
  auto changed = changePropertyCombo(IM_UNIQUE_ID, value);
  ImGui::SameLine();
  changed |= ::inspect(IM_UNIQUE_ID, value);
  ImGui::EndGroup();
  return changed;
}
bool inspect(TextureParam &value) {
  bool changed = false;

  auto *texture = value.texture.get();

  const auto aspectRatio =
    texture ? texture->getExtent().getAspectRatio() : 1.0f;
  const auto imageSize = glm::vec2{kTexturePreviewWidth,
                                   kTexturePreviewWidth * (1.0f / aspectRatio)};

  const auto defaultLabel = texture ? toString(texture->getType()) : "(none)";
  if (ImGui::TreeNode2(defaultLabel, imageSize.x)) {
    preview(texture, imageSize);
    if (ImGui::BeginDragDropTarget()) {
      if (auto incomingResource = extractResourceFromPayload(
            kImGuiPayloadTypeTexture, Services::Resources::Textures::value());
          incomingResource) {
        if (auto r = incomingResource->handle(); value.texture != r) {
          value.texture = std::move(r);
          changed = true;
        }
      }
      ImGui::EndDragDropTarget();
    }
    if (texture) {
      if (ImGui::IsKeyDown(ImGuiMod_Alt) && ImGui::IsItemHovered()) {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {8, 8});
        ImGui::SetTooltip("Format: %s", toString(texture->getPixelFormat()));
        ImGui::PopStyleVar();
      }
    }
    ImGui::TreePop();
  }
  ImGui::Dummy({kTexturePreviewWidth, 0});
  return changed;
}

template <typename E>
  requires std::is_scoped_enum_v<E>
void displayEnum(const E value) {
  static const ImColor kTypeColor{184, 215, 163};
  ImGui::TextColored(kTypeColor, "[%s]", toString(value));
}
template <typename E>
  requires std::is_scoped_enum_v<E>
bool inspect(E &value) {
  ImGui::BeginGroup();
  const auto changed =
    changeEnumCombo(IM_UNIQUE_ID, value, toString, ImGuiComboFlags_NoPreview);
  ImGui::SameLine();
  displayEnum(value);
  ImGui::EndGroup();
  return changed;
}

template <typename T>
bool inspectNode(const std::string &label, const VertexID id, T &value,
                 const std::unordered_set<VertexID> &connectedVertices) {
  ZoneTransientN(_tracy_zone, typeid(T).name(), true);

  auto nodeWidth =
    defaultTitleBar(!label.empty() ? label.c_str() : typeid(T).name());

  const auto changed = inspect(value);
  updateNodeWidth(nodeWidth);

  ImNodes::AddOutputAttribute(
    id, {
          .name = toString(getDataType(value)),
          .pinShape = kDefaultPinShape + connectedVertices.contains(id),
          .nodeWidth = nodeWidth,
        });

  return changed;
}
bool inspectNode(const std::string &label, const VertexID id,
                 TextureParam &value,
                 const std::unordered_set<VertexID> &connectedVertices) {
  ZoneScopedN("EmbeddedNode<Texture>");

  const auto texture = value.texture.get();

  const auto defaultLabel = texture ? toString(texture->getType()) : "Texture";
  auto nodeWidth = defaultTitleBar(!label.empty() ? label : defaultLabel,
                                   kTexturePreviewWidth);

  const auto changed = inspect(value);
  updateNodeWidth(nodeWidth);

  ImGui::Spacing();

  ImNodes::AddOutputAttribute(
    id, {
          .name = texture ? toString(getDataType(value)) : "out",
          .pinShape = kDefaultPinShape + connectedVertices.contains(id),
          .nodeWidth = nodeWidth,
        });
  return changed;
}

template <typename T>
void inspect(const char *name, EmbeddedNode<T> &node,
             const std::unordered_set<VertexID> &connectedVertices) {
  auto tempValue = node.value;
  const auto changed =
    bool(node.flags & NodeBase::Flags::Internal)
      ? inspect(tempValue)
      : inspectNode(!node.label.empty() ? node.label : name, node.vertex.id,
                    tempValue, connectedVertices);
  if (changed) node.setValue(std::move(tempValue));
}

[[nodiscard]] auto toString(const MatrixTransformNode::Space space) {
  switch (space) {
    using enum MatrixTransformNode::Space;

  case Local:
    return "Local";
  case World:
    return "World";
  case View:
    return "View";
  case Clip:
    return "Clip";
  case NDC:
    return "NDC";
  }
  assert(false);
  return "Undefined";
}

} // namespace

//
// NodeUIVisitor class:
//

void NodeUIVisitor::setContext(const ShaderGraph &g,
                               const gfx::Material::Surface *surface) {
  m_surface = surface;

  m_connectedVertices.clear();
  for (auto [_, c] : g.links() | std::views::filter(externalLink)) {
    m_connectedVertices.insert(c.source);
    m_connectedVertices.insert(c.target);
  }
}

void NodeUIVisitor::visit(EmbeddedNode<ValueVariant> &node) {
  inspect("Constant", node, m_connectedVertices);
}
void NodeUIVisitor::visit(EmbeddedNode<PropertyVariant> &node) {
  inspect("Property", node, m_connectedVertices);
}
void NodeUIVisitor::visit(EmbeddedNode<TextureParam> &node) {
  inspect("Texture", node, m_connectedVertices);
}

void NodeUIVisitor::visit(EmbeddedNode<Attribute> &node) {
  inspect("Attribute", node, m_connectedVertices);
}
void NodeUIVisitor::visit(EmbeddedNode<BuiltInConstant> &node) {
  inspect("BuiltInConstant", node, m_connectedVertices);
}
void NodeUIVisitor::visit(EmbeddedNode<BuiltInSampler> &node) {
  inspect("BuiltInSampler", node, m_connectedVertices);
}
void NodeUIVisitor::visit(EmbeddedNode<FrameBlockMember> &node) {
  inspect("FrameBlock::", node, m_connectedVertices);
}
void NodeUIVisitor::visit(EmbeddedNode<CameraBlockMember> &node) {
  inspect("CameraBlock::", node, m_connectedVertices);
}

void NodeUIVisitor::visit(AppendNode &node) {
  ZoneScopedN("AppendNode");

  auto nodeWidth = defaultTitleBar("Append");
  for (const auto &vertex : node.inputs) {
    auto *child = node.graph->getNode(vertex.vd);
    _addInputPin(*child, InputPinState::Active, InspectorMode::Inline);
    updateNodeWidth(nodeWidth);
  }
  ImGui::Separator2(nodeWidth);
  ImNodes::AddOutputAttribute(
    node.vertex.id, {
                      .name = "out",
                      .pinShape = kDefaultPinShape +
                                  m_connectedVertices.contains(node.vertex.id),
                      .nodeWidth = nodeWidth,
                    });
}
void NodeUIVisitor::visit(VectorSplitterNode &node) {
  ZoneScopedN("VectorSplitterNode");

  auto nodeWidth = defaultTitleBar(node);
  _addSimpleInputPin("in", node.vertex);
  updateNodeWidth(nodeWidth);

  ImGui::Separator2(nodeWidth);

  static const auto kSwizzleMaskSets = std::array{
    std::array{"xyz", "x", "y", "z", "w"},
    std::array{"rgb", "r", "g", "b", "a"},
    std::array{"stp", "s", "t", "p", "q"},
  };
  constexpr auto kNumPins = kSwizzleMaskSets.front().size();
  static const std::array<std::optional<ImU32>, kNumPins> kPins{
    std::nullopt,
    ImColor{1.0f, 0.2f, 0.321f},
    ImColor{0.545f, 0.862f, 0.0f},
    ImColor{0.156f, 0.563f, 1.0f},
    IM_COL32(255, 255, 255, 127),
  };
  assert(node.outputs.size() == kNumPins);

  for (const auto [vertex, color, label] :
       std::ranges::zip_view(node.outputs, kPins, kSwizzleMaskSets[0])) {
    ImNodes::AddOutputAttribute(
      vertex.id,
      {
        .name = label,
        .color = color,
        .pinShape = kDefaultPinShape + m_connectedVertices.contains(vertex.id),
        .nodeWidth = nodeWidth,
      });
  }
}
void NodeUIVisitor::visit(MatrixSplitterNode &node) {
  ZoneScopedN("MatrixSplitterNode");

  auto nodeWidth = defaultTitleBar(node);
  _addSimpleInputPin("in", node.vertex);
  updateNodeWidth(nodeWidth);

  ImGui::Separator2(nodeWidth);

  struct Item {
    const char *label;
    ImU32 color;
  };
  static const auto kPins = std::array{
    Item{"[0]", ImColor{1.0f, 0.2f, 0.321f}},
    Item{"[1]", ImColor{0.545f, 0.862f, 0.0f}},
    Item{"[2]", ImColor{0.156f, 0.563f, 1.0f}},
    Item{"[3]", IM_COL32(255, 255, 255, 127)},
  };
  assert(node.outputs.size() == kPins.size());

  for (auto [vertex, pin] : std::ranges::zip_view(node.outputs, kPins)) {
    ImNodes::AddOutputAttribute(
      vertex.id,
      {
        .name = pin.label,
        .color = pin.color,
        .pinShape = kDefaultPinShape + m_connectedVertices.contains(vertex.id),
        .nodeWidth = nodeWidth,
      });
  }
}
void NodeUIVisitor::visit(SwizzleNode &node) {
  ZoneScopedN("SwizzleNode");

  defaultTitleBar(node);
  _addSimpleInputPin("in", node.inputs.front());
  ImGui::SameLine();
  ImNodes::AddOutputAttribute(
    node.vertex.id,
    kDefaultPinShape + m_connectedVertices.contains(node.vertex.id), [&node] {
      constexpr auto kMaxSwizzleLength = 4;
      constexpr auto kWidthPerChar = 8.0f;
      ImGui::SetNextItemWidth(kWidthPerChar * kMaxSwizzleLength);
      const auto changed = ImGui::InputTextWithHint(
        "out", "Mask", &node.mask, ImGuiInputTextFlags_CallbackCharFilter,
        +[](ImGuiInputTextCallbackData *data) {
          if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter) {
            switch (data->EventChar) {
            case 'x':
            case 'y':
            case 'z':
            case 'w':
              break;

            default:
              data->EventChar = 0;
            }
          }

          const auto ud = static_cast<const std::string *>(data->UserData);
          assert(ud);

          return ud->length() < kMaxSwizzleLength ? 0 : -1;
        },
        &node.mask);
      if (changed) node.markDirty();
    });
}
void NodeUIVisitor::visit(ArithmeticNode &node) {
  ZoneScopedN("ArithmeticNode");

  ImNodes::BeginNodeTitleBar();
  ImGui::BeginGroup();
  if (changeEnumCombo(IM_UNIQUE_ID, node.operation, toString,
                      ImGuiComboFlags_NoPreview)) {
    node.markDirty();
  }
  ImGui::SameLine();
  ImGui::TextUnformatted(!node.label.empty() ? node.label.c_str()
                                             : toString(node.operation));
  ImGui::EndGroup();
  auto nodeWidth = ImGui::GetItemRectSize().x;
  ImNodes::EndNodeTitleBar();

  for (const auto [vd, _] : node.inputs) {
    _addInputPin(*node.graph->getNode(vd), InputPinState::Active,
                 InspectorMode::Inline);
    updateNodeWidth(nodeWidth);
  }
  ImGui::Separator2(nodeWidth);
  _addSimpleOutputPin("out", node.vertex, nodeWidth);
}
void NodeUIVisitor::visit(MatrixTransformNode &node) {
  ZoneScopedN("MatrixTransformNode");

  defaultTitleBar(node);
  _addSimpleInputPin("in", node.inputs.front().id);

  constexpr auto kComboWidth = 50.0f;
  ImGui::SameLine();
  ImGui::PushItemWidth(kComboWidth);
  auto changed = changeEnumCombo(IM_UNIQUE_ID, node.source, toString,
                                 ImGuiComboFlags_NoArrowButton);

  ImGui::SameLine();
  ImGui::TextUnformatted(ICON_FA_ARROW_RIGHT_LONG);

  ImGui::SameLine();
  changed |= changeEnumCombo(IM_UNIQUE_ID, node.target, toString,
                             ImGuiComboFlags_NoArrowButton);
  ImGui::PopItemWidth();
  if (changed) node.markDirty();

  ImGui::SameLine();
  _addSimpleOutputPin("out", node.vertex, 0);
}
void NodeUIVisitor::visit(TextureSamplingNode &node) {
  ZoneScopedN("TextureSamplingNode");

  assert(node.inputs.size() == 3);
  auto &g = *node.graph;

  auto nodeWidth = defaultTitleBar(node, kTexturePreviewWidth);
  for (auto i = 0; i < 2; ++i) {
    _addInputPin(*g.getNode(node.inputs[i].vd), InputPinState::Active,
                 InspectorMode::Inline);
    updateNodeWidth(nodeWidth);
  }
  {
    const auto [vd, id] = node.inputs[2];
    ImNodes::AddInputAttribute(
      id,
      {
        .name = "lod",
        .pinShape = kDefaultPinShape + m_connectedVertices.contains(id),
      },
      [&] {
        ImGui::SameLine();
        if (ImGui::Checkbox(IM_UNIQUE_ID, &node.useLOD)) {
          node.markDirty();
        }
        if (const auto inspectLod = g.countOutEdges(vd) == 0; inspectLod) {
          ImGui::SameLine();
          g.getNode(vd)->accept(*this);
        }
      });
    updateNodeWidth(nodeWidth);
  }
  ImGui::Separator2(nodeWidth);
  _addSimpleOutputPin("out", node.vertex, nodeWidth);
}
void NodeUIVisitor::visit(ScriptedNode &node) {
  const auto *data = node.scriptedFunction.data;
  if (!data) return;

  ZoneScopedN("ScriptedNode");

  auto nodeWidth = defaultTitleBar(node);
  for (const auto [i, vertex] : node.inputs | std::views::enumerate) {
    const auto &arg =
      data ? data->args[i]
           : ScriptedFunction::Data::Parameter{.name = std::format("arg{}", i)};

    _addInputPin(*node.graph->getNode(vertex.vd), InputPinState::Active,
                 InspectorMode::Inline);
    updateNodeWidth(nodeWidth);

    if (ImGui::IsKeyDown(ImGuiMod_Alt) && ImGui::IsItemHovered())
      ImGui::ShowTooltip(arg.description);
  }
  if (!node.inputs.empty()) {
    ImGui::Separator2(nodeWidth);
  }
  _addSimpleOutputPin("out", node.vertex, nodeWidth);
}
void NodeUIVisitor::visit(CustomNode &node) {
  const auto *data = node.userFunction.data;
  if (!data) return;

  ZoneScopedN("CustomNode");

  auto nodeWidth = defaultTitleBar(node);
  if (ImGui::IsKeyDown(ImGuiMod_Alt) && ImGui::IsItemHovered())
    ImGui::ShowTooltip(data->code.c_str());

  for (const auto &[vertex, arg] :
       std::ranges::zip_view(node.inputs, data->inputs)) {
    _addInputPin(*node.graph->getNode(vertex.vd), InputPinState::Active,
                 InspectorMode::Inline);
    updateNodeWidth(nodeWidth);
  }
  if (!node.inputs.empty()) {
    ImGui::Separator2(nodeWidth);
  }
  _addSimpleOutputPin(toString(data->output), node.vertex, nodeWidth);
}

void NodeUIVisitor::visit(VertexMasterNode &node) {
  ZoneScopedN("VertexMasterNode");

  defaultTitleBar("VertexMaster");
  for (const auto [i, vertex] : node.inputs | std::views::enumerate) {
    _addInputPin(*node.graph->getNode(vertex.vd), InputPinState::Active,
                 InspectorMode::Popup);
  }
}
void NodeUIVisitor::visit(SurfaceMasterNode &node) {
  ZoneScopedN("SurfaceMasterNode");

  defaultTitleBar("SurfaceMaster");
  for (const auto [i, vertex] : node.inputs | std::views::enumerate) {
    const auto isAvailable = SurfaceMasterNode::kFields[i].isAvailable;
    _addInputPin(*node.graph->getNode(vertex.vd),
                 isAvailable(*m_surface) ? InputPinState::Active
                                         : InputPinState::Inactive,
                 InspectorMode::Popup);
  }
}
void NodeUIVisitor::visit(PostProcessMasterNode &node) {
  ZoneScopedN("PostProcessMasterNode");

  defaultTitleBar("PostProcessMaster");
  for (const auto [i, vertex] : node.inputs | std::views::enumerate) {
    _addInputPin(*node.graph->getNode(vertex.vd), InputPinState::Active,
                 InspectorMode::Popup);
  }
}

//
// (private):
//

void NodeUIVisitor::_addSimpleInputPin(const std::string_view label,
                                       const VertexID id) {
  ImNodes::AddInputAttribute(
    id, {
          .name = label,
          .pinShape = kDefaultPinShape + m_connectedVertices.contains(id),
        });
}
void NodeUIVisitor::_addSimpleOutputPin(const std::string_view label,
                                        const VertexID id,
                                        const float nodeWidth) {
  ImNodes::AddOutputAttribute(
    id, {
          .name = label,
          .pinShape = kDefaultPinShape + m_connectedVertices.contains(id),
          .nodeWidth = nodeWidth,
        });
}

void NodeUIVisitor::_addInputPin(NodeBase &node, const InputPinState state,
                                 const InspectorMode inspectorMode) {
  assert(bool(node.flags & NodeBase::Flags::Input));

  const auto active = std::to_underlying(state);
  const auto hasLink = m_connectedVertices.contains(node.vertex);
  ImNodes::AddInputAttribute(
    node.vertex,
    {
      .name = node.label,
      .pinShape =
        active ? std::make_optional(kDefaultPinShape + hasLink) : std::nullopt,
    },
    [&] {
      if (active && !hasLink) {
        if (!node.label.empty() &&
            typeid(node) != typeid(EmbeddedNode<TextureParam>)) {
          ImGui::SameLine();
        }
        switch (inspectorMode) {
        case InspectorMode::Inline:
          node.accept(*this);
          break;
        case InspectorMode::Popup: {
          constexpr auto kPopupId = IM_UNIQUE_ID;
          if (ImGui::SmallButton(ICON_FA_PENCIL)) ImGui::OpenPopup(kPopupId);

          ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{4.0f, 4.0f});
          if (ImGui::BeginPopup(kPopupId, ImGuiWindowFlags_NoMove)) {
            node.accept(*this);
            ImGui::EndPopup();
          }
          ImGui::PopStyleVar();
        } break;
        }
      }
    });
}
