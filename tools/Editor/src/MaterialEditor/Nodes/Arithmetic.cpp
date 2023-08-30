#include "MaterialEditor/Nodes/Arithmetic.hpp"
#include "NodesInternal.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"
#include "MaterialEditor/ChangeEnumCombo.hpp"

namespace {

[[nodiscard]] auto makeHash(DataType lhs, ArithmeticNode::Operation op,
                            DataType rhs) {
  // | 6 bits | 3 bits | 6 bits |
  // |   0    |  6..8  |  9..14 |
  // |  lhs   |   op   |  rhs   |
#if 0
  uint32_t h{0};
  h = glm::bitfieldInsert(h, static_cast<uint32_t>(lhs), 0, 6);
  h = glm::bitfieldInsert(h, static_cast<uint32_t>(op), 6, 3);
  h = glm::bitfieldInsert(h, static_cast<uint32_t>(rhs), 9, 6);
  return h;
#else
  return (static_cast<uint32_t>(rhs) << 9) | (static_cast<uint32_t>(op) << 6) |
         static_cast<uint32_t>(lhs);
#endif
}
[[nodiscard]] auto buildOperations() {
  std::unordered_map<uint32_t, DataType> m;

  const auto addOperation = [&m](ArithmeticNode::Operation op,
                                 std::pair<DataType, DataType> p, DataType rv) {
    const auto h = makeHash(p.first, op, p.second);
    auto [_, inserted] = m.try_emplace(h, rv);
    assert(inserted);
  };
  const auto addOperations =
    [&addOperation](std::initializer_list<ArithmeticNode::Operation> ops,
                    std::pair<DataType, DataType> p, DataType rv) {
      for (const auto op : ops)
        addOperation(op, p, rv);
    };

  using enum ArithmeticNode::Operation;
  const auto addAllOperations =
    [&addOperations](std::pair<DataType, DataType> p, DataType rv) {
      addOperations({Add, Subtract, Multiply, Divide}, p, rv);
    };

  using enum DataType;

#pragma region UInt32
  addAllOperations({UInt32, UVec2}, UVec2);
  addAllOperations({UInt32, UVec3}, UVec3);
  addAllOperations({UInt32, UVec4}, UVec4);
  addAllOperations({UInt32, Int32}, UInt32);
  addAllOperations({UInt32, IVec2}, UVec2);
  addAllOperations({UInt32, IVec3}, UVec3);
  addAllOperations({UInt32, IVec4}, UVec4);
  addAllOperations({UInt32, Float}, Float);
  addAllOperations({UInt32, Vec2}, Vec2);
  addAllOperations({UInt32, Vec3}, Vec3);
  addAllOperations({UInt32, Vec4}, Vec4);
  addAllOperations({UInt32, Double}, Double);
  addAllOperations({UInt32, DVec2}, DVec2);
  addAllOperations({UInt32, DVec3}, DVec3);
  addAllOperations({UInt32, DVec4}, DVec4);
#pragma endregion
#pragma region UVec2
  // Impossible conversions: uvec2 -> *vec3/ *vec4
  addAllOperations({UVec2, UInt32}, UVec2);
  addAllOperations({UVec2, Int32}, UVec2);
  addAllOperations({UVec2, IVec2}, UVec2);
  addAllOperations({UVec2, Float}, Vec2);
  addAllOperations({UVec2, Vec2}, Vec2);
  addAllOperations({UVec2, Double}, DVec2);
  addAllOperations({UVec2, DVec2}, DVec2);
#pragma endregion
#pragma region UVec3
  // Impossible conversions: uvec3 -> *vec2/ *vec4
  addAllOperations({UVec3, UInt32}, UVec3);
  addAllOperations({UVec3, Int32}, UVec3);
  addAllOperations({UVec3, IVec3}, UVec3);
  addAllOperations({UVec3, Float}, Vec3);
  addAllOperations({UVec3, Vec3}, Vec3);
  addAllOperations({UVec3, Double}, DVec3);
  addAllOperations({UVec3, DVec3}, DVec3);
#pragma endregion
#pragma region UVec4
  // Impossible conversions: uvec4 -> *vec2/ *vec3
  addAllOperations({UVec4, UInt32}, UVec4);
  addAllOperations({UVec4, Int32}, UVec4);
  addAllOperations({UVec4, IVec4}, UVec4);
  addAllOperations({UVec4, Float}, Vec4);
  addAllOperations({UVec4, Vec4}, Vec4);
  addAllOperations({UVec4, Double}, DVec4);
  addAllOperations({UVec4, DVec4}, DVec4);
#pragma endregion

#pragma region Int32
  addAllOperations({Int32, UInt32}, UInt32);
  addAllOperations({Int32, UVec2}, UVec2);
  addAllOperations({Int32, UVec3}, UVec3);
  addAllOperations({Int32, UVec4}, UVec4);
  addAllOperations({Int32, IVec2}, IVec2);
  addAllOperations({Int32, IVec3}, IVec3);
  addAllOperations({Int32, IVec4}, IVec4);
  addAllOperations({Int32, Float}, Float);
  addAllOperations({Int32, Vec2}, Vec2);
  addAllOperations({Int32, Vec3}, Vec3);
  addAllOperations({Int32, Vec4}, Vec4);
  addAllOperations({Int32, Double}, Double);
  addAllOperations({Int32, DVec2}, DVec2);
  addAllOperations({Int32, DVec3}, DVec3);
  addAllOperations({Int32, DVec4}, DVec4);
#pragma endregion
#pragma region IVec2
  // Impossible conversions: ivec2 -> *vec3/ *vec4
  addAllOperations({IVec2, UInt32}, UVec2);
  addAllOperations({IVec2, UVec2}, UVec2);
  addAllOperations({IVec2, Int32}, IVec2);
  addAllOperations({IVec2, Float}, Vec2);
  addAllOperations({IVec2, Vec2}, Vec2);
  addAllOperations({IVec2, Double}, DVec2);
  addAllOperations({IVec2, DVec2}, DVec2);
#pragma endregion
#pragma region IVec3
  // Impossible conversions: uvec3 -> *vec2/ *vec4
  addAllOperations({IVec3, UInt32}, UVec3);
  addAllOperations({IVec3, UVec3}, UVec3);
  addAllOperations({IVec3, Int32}, IVec3);
  addAllOperations({IVec3, Float}, Vec3);
  addAllOperations({IVec3, Vec3}, Vec3);
  addAllOperations({IVec3, Double}, DVec3);
  addAllOperations({IVec3, DVec3}, DVec3);
#pragma endregion
#pragma region IVec4
  // Impossible conversions: ivec4 -> *vec2/ *vec3
  addAllOperations({IVec4, UInt32}, UVec4);
  addAllOperations({IVec4, UVec4}, UVec4);
  addAllOperations({IVec4, Int32}, IVec4);
  addAllOperations({IVec4, Float}, Vec4);
  addAllOperations({IVec4, Vec4}, Vec4);
  addAllOperations({IVec4, Double}, DVec4);
  addAllOperations({IVec4, DVec4}, DVec4);
#pragma endregion

#pragma region Float
  addAllOperations({Float, UInt32}, Float);
  addAllOperations({Float, UVec2}, Vec2);
  addAllOperations({Float, UVec3}, Vec3);
  addAllOperations({Float, UVec4}, Vec4);
  addAllOperations({Float, Int32}, Float);
  addAllOperations({Float, IVec2}, Vec2);
  addAllOperations({Float, IVec3}, Vec3);
  addAllOperations({Float, IVec4}, Vec4);
  addAllOperations({Float, Vec2}, Vec2);
  addAllOperations({Float, Vec3}, Vec3);
  addAllOperations({Float, Vec4}, Vec4);
  addAllOperations({Float, Double}, Double);
  addAllOperations({Float, DVec2}, DVec2);
  addAllOperations({Float, DVec3}, DVec3);
  addAllOperations({Float, DVec4}, DVec4);
#pragma endregion
#pragma region Vec2
  // Impossible conversions: vec2 -> *vec3/ *vec4
  addAllOperations({Vec2, UInt32}, Vec2);
  addAllOperations({Vec2, UVec2}, Vec2);
  addAllOperations({Vec2, Int32}, Vec2);
  addAllOperations({Vec2, IVec2}, Vec2);
  addAllOperations({Vec2, Float}, Vec2);
  addAllOperations({Vec2, Double}, DVec2);
  addAllOperations({Vec2, DVec2}, DVec2);
#pragma endregion
#pragma region Vec3
  // Impossible conversions: vec3 -> *vec2/ *vec4
  addAllOperations({Vec3, UInt32}, Vec3);
  addAllOperations({Vec3, UVec3}, Vec3);
  addAllOperations({Vec3, Int32}, Vec3);
  addAllOperations({Vec3, IVec3}, Vec3);
  addAllOperations({Vec3, Float}, Vec3);
  addAllOperations({Vec3, Double}, DVec3);
  addAllOperations({Vec3, DVec3}, DVec3);
#pragma endregion
#pragma region Vec4
  // Impossible conversions: vec4 -> *vec2/ *vec3
  addAllOperations({Vec4, UInt32}, Vec4);
  addAllOperations({Vec4, UVec4}, Vec4);
  addAllOperations({Vec4, Int32}, Vec4);
  addAllOperations({Vec4, IVec4}, Vec4);
  addAllOperations({Vec4, Float}, Vec4);
  addAllOperations({Vec4, Double}, DVec4);
  addAllOperations({Vec4, DVec4}, DVec4);
#pragma endregion

#pragma region Double
  addAllOperations({Double, UInt32}, Double);
  addAllOperations({Double, UVec2}, DVec2);
  addAllOperations({Double, UVec3}, DVec3);
  addAllOperations({Double, UVec4}, DVec4);
  addAllOperations({Double, Int32}, Double);
  addAllOperations({Double, IVec2}, DVec2);
  addAllOperations({Double, IVec3}, DVec3);
  addAllOperations({Double, IVec4}, DVec4);
  addAllOperations({Double, Float}, Double);
  addAllOperations({Double, Vec2}, DVec2);
  addAllOperations({Double, Vec3}, DVec3);
  addAllOperations({Double, Vec4}, DVec4);
  addAllOperations({Double, DVec2}, DVec2);
  addAllOperations({Double, DVec3}, DVec3);
  addAllOperations({Double, DVec4}, DVec4);
#pragma endregion
#pragma region DVec2
  // Impossible conversions: dvec2 -> *vec3/ *vec4
  addAllOperations({DVec2, UInt32}, DVec2);
  addAllOperations({DVec2, UVec2}, DVec2);
  addAllOperations({DVec2, Int32}, DVec2);
  addAllOperations({DVec2, IVec2}, DVec2);
  addAllOperations({DVec2, Float}, DVec2);
  addAllOperations({DVec2, Vec2}, DVec2);
  addAllOperations({DVec2, Double}, DVec2);
#pragma endregion
#pragma region DVec3
  // Impossible conversions: dvec3 -> *vec2/ *vec4
  addAllOperations({DVec3, UInt32}, DVec3);
  addAllOperations({DVec3, UVec3}, DVec3);
  addAllOperations({DVec3, Int32}, DVec3);
  addAllOperations({DVec3, IVec3}, DVec3);
  addAllOperations({DVec3, Float}, DVec3);
  addAllOperations({DVec3, Vec3}, DVec3);
  addAllOperations({DVec3, Double}, DVec3);
#pragma endregion
#pragma region DVec4
  // Impossible conversions: dvec4 -> *vec2/ *vec3
  addAllOperations({DVec4, UInt32}, DVec4);
  addAllOperations({DVec4, UVec4}, DVec4);
  addAllOperations({DVec4, Int32}, DVec4);
  addAllOperations({DVec4, IVec4}, DVec4);
  addAllOperations({DVec4, Float}, DVec4);
  addAllOperations({DVec4, Vec4}, DVec4);
  addAllOperations({DVec4, Double}, DVec4);
#pragma endregion

#pragma region Mat2
  addOperations({Add, Subtract}, {Mat2, Float}, Mat2);
  addOperations({Add, Subtract}, {Float, Mat2}, Mat2);

  addOperations({Multiply, Divide}, {Mat2, Float}, Mat2);
  addOperations({Multiply, Divide}, {Float, Mat2}, Mat2);
  addOperation(Multiply, {Mat2, Vec2}, Vec2);
  addOperation(Multiply, {Vec2, Mat2}, Vec2);
#pragma endregion
#pragma region Mat3
  addOperations({Add, Subtract}, {Mat3, Float}, Mat3);
  addOperations({Add, Subtract}, {Float, Mat3}, Mat3);

  addOperations({Multiply, Divide}, {Mat3, Float}, Mat3);
  addOperations({Multiply, Divide}, {Float, Mat3}, Mat3);
  addOperation(Multiply, {Mat3, Vec3}, Vec3);
  addOperation(Multiply, {Vec3, Mat3}, Vec3);
#pragma endregion
#pragma region Mat4
  addOperations({Add, Subtract}, {Mat4, Float}, Mat4);
  addOperations({Add, Subtract}, {Float, Mat4}, Mat4);

  addOperations({Multiply, Divide}, {Mat4, Float}, Mat4);
  addOperations({Multiply, Divide}, {Float, Mat4}, Mat4);
  addOperation(Multiply, {Mat4, Vec4}, Vec4);
  addOperation(Multiply, {Vec4, Mat4}, Vec4);
#pragma endregion

  return m;
}

[[nodiscard]] auto getReturnType(DataType lhs, ArithmeticNode::Operation op,
                                 DataType rhs) {
  using enum DataType;
  if (lhs == Undefined || rhs == Undefined)
    return Undefined;
  else if (lhs == rhs)
    return lhs;

  static const auto m = buildOperations();
  const auto it = m.find(makeHash(lhs, op, rhs));
  return it != m.cend() ? it->second : Undefined;
}

[[nodiscard]] constexpr auto getOperationToken(ArithmeticNode::Operation op) {
  switch (op) {
    using enum ArithmeticNode::Operation;
  case Add:
    return "+";
  case Subtract:
    return "-";
  case Multiply:
    return "*";
  case Divide:
    return "/";
  }
  assert(false);
  return "?";
}

} // namespace

//
// ArithmeticNode struct:
//

ArithmeticNode ArithmeticNode::create(ShaderGraph &g, VertexDescriptor parent,
                                      Operation op) {
  return {
    .input =
      {
        .lhs =
          createInternalInput(g, parent, "x", ValueVariant{glm::vec3{0.0f}}),
        .rhs =
          createInternalInput(g, parent, "y", ValueVariant{glm::vec3{0.0f}}),
      },
    .operation = op,
  };
}
ArithmeticNode ArithmeticNode::clone(ShaderGraph &g,
                                     VertexDescriptor parent) const {
  auto node = create(g, parent, operation);
  copySimpleVariant(g, input.lhs, node.input.lhs);
  copySimpleVariant(g, input.rhs, node.input.rhs);
  return node;
}

void ArithmeticNode::remove(ShaderGraph &g) {
  removeVertices(g, {input.lhs, input.rhs});
}

bool ArithmeticNode::inspect(ShaderGraph &g, int32_t id) {
  ImNodes::BeginNodeTitleBar();
  ImGui::BeginGroup();
  auto changed = changeEnumCombo(IM_UNIQUE_ID, operation, toString,
                                 ImGuiComboFlags_NoPreview);
  ImGui::SameLine();
  const auto label = toString(operation);
  ImGui::TextUnformatted(label);
  ImGui::EndGroup();
  const auto nodeWidth = ImGui::GetItemRectSize().x;
  ImNodes::EndNodeTitleBar();

  // -- input:

  changed |= addInputPin(g, input.lhs, {.name = "A"}, InspectorMode::Inline);
  auto maxWidth = ImGui::GetItemRectSize().x;

  changed |= addInputPin(g, input.rhs, {.name = "B"}, InspectorMode::Inline);
  maxWidth = glm::max(maxWidth, ImGui::GetItemRectSize().x);

  // -- output:

  ImNodes::AddOutputAttribute(
    id, {.name = "out", .nodeWidth = glm::max(nodeWidth, maxWidth)});

  return changed;
}
NodeResult ArithmeticNode::evaluate(MaterialGenerationContext &context,
                                    int32_t id) const {
  auto &[_, tokens, composer] = *context.currentShader;

  auto [lhsArg, rhsArg] = extract<2>(tokens);
  if (auto returnType =
        getReturnType(lhsArg.dataType, operation, rhsArg.dataType);
      returnType != DataType::Undefined) {
    ShaderToken token{
      .name = nodeIdToString(id),
      .dataType = returnType,
    };
    composer.addVariable(token.dataType, token.name,
                         std::format("{} {} {}", lhsArg.name,
                                     getOperationToken(operation),
                                     rhsArg.name));
    return token;
  } else {
    return std::unexpected{
      std::format("Could not perform arithmetic operation: ({} {} {}).",
                  toString(lhsArg.dataType), getOperationToken(operation),
                  toString(rhsArg.dataType)),
    };
  }
}

//
// Helper:
//

const char *toString(ArithmeticNode::Operation op) {
#define CASE(Value)                                                            \
  case ArithmeticNode::Operation::Value:                                       \
    return #Value

  switch (op) {
    CASE(Add);
    CASE(Subtract);
    CASE(Multiply);
    CASE(Divide);
  }
#undef CASE

  assert(false);
  return "Undefined";
}
