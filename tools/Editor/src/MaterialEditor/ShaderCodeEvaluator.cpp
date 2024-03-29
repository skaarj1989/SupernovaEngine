#include "MaterialEditor/ShaderCodeEvaluator.hpp"

#include "StringUtility.hpp" // join

#include "MaterialEditor/ValueVariant.hpp"
#include "MaterialEditor/PropertyVariant.hpp"
#include "MaterialEditor/TextureParam.hpp"
#include "MaterialEditor/Attribute.hpp"
#include "MaterialEditor/BuiltInConstants.hpp"
#include "MaterialEditor/FrameBlockMember.hpp"
#include "MaterialEditor/CameraBlockMember.hpp"

#include "MaterialEditor/SplitVector.hpp"
#include "MaterialEditor/SplitMatrix.hpp"

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

#include "MaterialEditor/ShaderGraph.hpp"

namespace {

[[nodiscard]] bool shouldSkip(const NodeBase &node) {
  return bool(node.flags & NodeBase::Flags::Input) && node.hasConnectedInput();
}

[[nodiscard]] std::optional<DataType> combine(const DataType a,
                                              const DataType b) {
  const auto baseType = isVector(a) ? getBaseDataType(a) : a;
  const auto outVectorSize = countChannels(a) + countChannels(b);
  return outVectorSize > 0 && outVectorSize <= 4
           ? std::optional{constructVectorType(baseType, outVectorSize)}
           : std::nullopt;
}
[[nodiscard]] std::optional<DataType> combine(const ShaderToken &a,
                                              const ShaderToken &b) {
  const auto qualified = [](const ShaderToken &token) {
    return token.isValid() &&
           (isScalar(token.dataType) || isVector(token.dataType));
  };
  return qualified(a) && qualified(b) ? combine(a.dataType, b.dataType)
                                      : std::nullopt;
}

const char *getPostfix(const SplitVector e) {
  // clang-format off
  switch (e) {
    using enum SplitVector;
  
  case X: return ".x";
  case Y: return ".y";
  case Z: return ".z";
  case W: return ".w";
  
  case XYZ: return ".xyz";
  }
  // clang-format on

  assert(false);
  return "";
}

[[nodiscard]] constexpr auto getNumRequiredChannels(const SplitVector e) {
  switch (e) {
    using enum SplitVector;

  case R:
  case G:
    return 2;
  case B:
  case RGB:
    return 3;
  case A:
    return 4;
  }

  assert(false);
  return 0;
}
[[nodiscard]] constexpr auto getNumRequiredColumns(const SplitMatrix e) {
  switch (e) {
    using enum SplitMatrix;

  case Column0:
  case Column1:
    return 2;
  case Column2:
    return 3;
  case Column3:
    return 4;
  }

  assert(false);
  return 0;
}

[[nodiscard]] auto sameTypes(std::span<const DataType> a,
                             std::span<const UserFunction::Data::Parameter> b) {
  assert(a.size() == b.size());
  return std::ranges::equal(a, b, [](const DataType lhs, const auto &rhs) {
    return lhs == rhs.dataType;
  });
}

[[nodiscard]] auto getRequiredUVType(const DataType type) {
  using enum DataType;
  switch (type) {
  case Sampler2D:
    return Vec2;
  case SamplerCube:
    return Vec3;

  default:
    return Undefined;
  }
}

[[nodiscard]] constexpr auto makeHash(const MatrixTransformNode::Space lhs,
                                      const MatrixTransformNode::Space rhs) {
  return std::to_underlying(lhs) << 16 | std::to_underlying(rhs);
}
[[nodiscard]] std::pair<DataType, const char *>
findFunction(const MatrixTransformNode::Space source,
             const MatrixTransformNode::Space target) {
  using enum DataType;
  switch (makeHash(source, target)) {
    using enum MatrixTransformNode::Space;

  case makeHash(World, View):
    return {Vec3, "worldToView"};
  case makeHash(World, Clip):
    return {Vec4, "worldToClip"};
  case makeHash(World, NDC):
    return {Vec3, "worldToNDC"};

  case makeHash(View, World):
    return {Vec3, "viewToWorld"};
  case makeHash(View, Clip):
    return {Vec4, "viewToClip"};
  case makeHash(View, NDC):
    return {Vec3, "viewToNDC"};

    /*
  case makeHash(Clip, World):
    return {Vec4, "clipToWorld"};
  case makeHash(Clip, View):
    return {Vec4, "clipToView"};
    */

  case makeHash(NDC, View):
    return {Vec3, "NDCToView"};
  case makeHash(NDC, World):
    return {Vec3, "NDCToWorld"};
  }
  return {Undefined, "(not found)"};
}

[[nodiscard]] auto
makePropertyNameCandidates(const rhi::ShaderType shaderType, const VertexID id,
                           const std::string_view userLabel) {
  std::pair<std::string, std::string> identifiers;
  if (!userLabel.empty()) {
    identifiers.first =
      std::format("{}_{}", getPrefix(shaderType),
                  ShaderCodeComposer::makeIdentifier(userLabel.data()));
  }
  identifiers.second = std::format("{}_id_{}", getPrefix(shaderType), id);
  return identifiers;
}
[[nodiscard]] auto
makeTextureNameCandidates(const EmbeddedNode<TextureParam> &node) {
  std::pair<std::string, std::string> identifiers;
  if (bool(node.flags & NodeBase::Flags::Output) && !node.label.empty()) {
    identifiers.first =
      std::format("t_{}_{}", getPrefix(node.getOrigin()),
                  ShaderCodeComposer::makeIdentifier(node.label));
  }
  identifiers.second =
    std::format("t_{}_{}", getPrefix(node.getOrigin()), node.vertex.id);
  return identifiers;
}

template <typename T, typename V>
std::string emplaceWithFallback(
  T &map, const std::pair<std::string, std::string> &identifiers, V &&v) {
  for (auto &key : {identifiers.first, identifiers.second}) {
    if (key.empty()) {
      continue;
    } else if (map.contains(key)) {
      return key;
    } else {
      [[maybe_unused]] auto [_, emplaced] =
        map.try_emplace(key, std::forward<V>(v));
      assert(emplaced);
      return key;
    }
  }
  assert(false);
  return "";
}

} // namespace

//
// ShaderCodeEvaluator class:
//

ShaderCodeEvaluator::Result
ShaderCodeEvaluator::evaluate(const ShaderGraph &g) {
  _clear();
  try {
    auto vertices = g.getExecutionOrder();
    while (!vertices.empty()) {
      const auto vd = vertices.top();
      vertices.pop();

      auto *node = g.getNode(vd);
      if (!shouldSkip(*node)) node->accept(*this);
    }
  } catch (const std::exception &e) {

    return std::unexpected{e.what()};
  }
  return ShaderDefinition{
    .code = m_composer.getCode(),
    .properties = std::move(m_properties),
    .textures = std::move(m_textures),
    .usedUserFunctions = {m_usedUserFunctions.begin(),
                          m_usedUserFunctions.end()},
  };
}

//
// (private):
//

void ShaderCodeEvaluator::_clear() {
  m_tokens = {};
  m_composer.clear();
  m_properties.clear();
  m_textures.clear();
  m_usedUserFunctions.clear();
}

void ShaderCodeEvaluator::visit(const EmptyNode &) {
  m_tokens.push({.dataType = DataType::Undefined});
}

void ShaderCodeEvaluator::visit(const EmbeddedNode<ValueVariant> &node) {
  auto name = nodeIdToString(node.vertex.id);
  m_composer.addVariable(name, node.value);
  m_tokens.push({
    .name = std::move(name),
    .dataType = getDataType(node.value),
  });
}
void ShaderCodeEvaluator::visit(const EmbeddedNode<PropertyVariant> &node) {
  const auto name = emplaceWithFallback(
    m_properties,
    makePropertyNameCandidates(node.getOrigin(), node.vertex.id, node.label),
    node.value);
  m_tokens.push({
    .name = std::format("properties.{}", name),
    .dataType = getDataType(node.value),
  });
}
void ShaderCodeEvaluator::visit(const EmbeddedNode<TextureParam> &node) {
  const auto dataType = getDataType(node.value);
  if (dataType == DataType::Undefined)
    throw NodeEvaluationException{node, "Missing texture."};

  m_tokens.push({
    .name = emplaceWithFallback(m_textures, makeTextureNameCandidates(node),
                                node.value.texture),
    .dataType = dataType,
  });
}

void ShaderCodeEvaluator::visit(const EmbeddedNode<SplitVector> &node) {
  const auto token = extractTop(m_tokens);
  if (countChannels(token.dataType) < getNumRequiredChannels(node.value)) {
    throw NodeEvaluationException{node, "Not enough channels."};
  }

  const auto baseType = getBaseDataType(token.dataType);
  const auto toSingleChannel = node.value != SplitVector::XYZ;
  m_tokens.push({
    .name = token.name + getPostfix(node.value),
    .dataType = toSingleChannel ? baseType : constructVectorType(baseType, 3),
  });
}
void ShaderCodeEvaluator::visit(const EmbeddedNode<SplitMatrix> &node) {
  const auto token = extractTop(m_tokens);
  const auto numColumns = countColumns(token.dataType);
  if (numColumns < getNumRequiredColumns(node.value)) {
    throw NodeEvaluationException{node, "Not enough columns."};
  }

  const auto baseType = getBaseDataType(token.dataType);
  const auto columnIdx = std::to_underlying(node.value);
  m_tokens.push({
    .name = std::format("{}[{}]", token.name, columnIdx),
    .dataType = constructVectorType(baseType, numColumns),
  });
}

void ShaderCodeEvaluator::visit(const AppendNode &node) {
  auto [aArg, bArg] = extract<2>(m_tokens);
  const auto dataType = combine(aArg, bArg);
  if (!dataType) {
    throw NodeEvaluationException{node, std::format("Can't append: {} to {}.",
                                                    toString(bArg.dataType),
                                                    toString(aArg.dataType))};
  }

  ShaderToken token{
    .name = nodeIdToString(node.vertex.id),
    .dataType = *dataType,
  };
  m_composer.addVariable(
    token.dataType, token.name,
    std::format("{}({}, {})", toString(token.dataType), aArg.name, bArg.name));
  m_tokens.push(std::move(token));
}
void ShaderCodeEvaluator::visit(const VectorSplitterNode &node) {
  if (m_tokens.empty()) throw NodeEvaluationException{node, "No input."};

  if (const auto &arg = m_tokens.top();
      !arg.isValid() || !isVector(arg.dataType)) {
    throw NodeEvaluationException{
      node, std::format("Cant split: {}.", toString(arg.dataType))};
  }
}
void ShaderCodeEvaluator::visit(const MatrixSplitterNode &node) {
  if (m_tokens.empty()) throw NodeEvaluationException{node, "No input."};

  if (const auto &arg = m_tokens.top();
      !arg.isValid() || !isMatrix(arg.dataType)) {
    throw NodeEvaluationException{
      node, std::format("Cant split: {}.", toString(arg.dataType))};
  }
}
void ShaderCodeEvaluator::visit(const SwizzleNode &node) {
  const auto arg = extractTop(m_tokens);
  if (!arg.isValid() ||
      !SwizzleNode::isMaskValid(node.mask, countChannels(arg.dataType))) {
    throw NodeEvaluationException{
      node,
      std::format("Can't swizzle: {}.{}", toString(arg.dataType), node.mask)};
  }

  const auto baseType = getBaseDataType(arg.dataType);
  m_tokens.push({
    .name = std::format("{}.{}", arg.name, node.mask),
    .dataType =
      constructVectorType(baseType, static_cast<uint32_t>(node.mask.length())),
  });
}
void ShaderCodeEvaluator::visit(const MatrixTransformNode &node) {
  if (node.source == node.target) {
    throw NodeEvaluationException{node, "Do not use noop nodes."};
  }

  const auto arg = extractTop(m_tokens);
  if (!arg.isValid() || arg.dataType != DataType::Vec4) {
    throw NodeEvaluationException{node, "Invalid input. Expected Vec4."};
  }

  auto [returnType, fn] = findFunction(node.source, node.target);
  if (returnType == DataType::Undefined) {
    throw NodeEvaluationException{node, "Could not find a function."};
  }

  ShaderToken token{
    .name = nodeIdToString(node.vertex.id),
    .dataType = returnType,
  };
  m_composer.addVariable(token.dataType, token.name,
                         buildFunctionCall(fn, {arg.name}));
  m_tokens.push(std::move(token));
}
void ShaderCodeEvaluator::visit(const TextureSamplingNode &node) {
  // https://registry.khronos.org/OpenGL-Refpages/gl4/html/texture.xhtml
  auto [textureArg, uvArg, lodArg] = extract<3>(m_tokens);
  if (!isSampler(textureArg.dataType)) {
    throw NodeEvaluationException{node, "Not a texture."};
  }
  const auto uvStr = assure(uvArg, getRequiredUVType(textureArg.dataType));
  if (!uvStr) {
    throw NodeEvaluationException{node, "Invalid UV argument."};
  }
  ShaderToken token{
    .name = nodeIdToString(node.vertex.id),
    .dataType = DataType::Vec4,
  };
  if (node.useLOD) {
    const auto lodStr = assure(lodArg, DataType::Float);
    if (!lodStr) {
      throw NodeEvaluationException{node, "Invalid LOD argument."};
    }
    m_composer.addVariable(
      token.dataType, token.name,
      std::format("textureLod({}, {}, {})", textureArg.name, *uvStr, *lodStr));
  } else {
    m_composer.addVariable(
      token.dataType, token.name,
      std::format("texture({}, {})", textureArg.name, *uvStr));
  }
  m_tokens.push(std::move(token));
}
void ShaderCodeEvaluator::visit(const ScriptedNode &node) {
  const auto *data = node.scriptedFunction.data;
  if (!data) {
    throw NodeEvaluationException{node, "Corrupted scripted function."};
  }

  std::vector<ShaderToken> args;
  if (!node.inputs.empty()) args = extractN(m_tokens, node.inputs.size());
  const auto argTypes = extractArgTypes(args);
  const auto returnType = data->getReturnTypeCb(argTypes);
  if (returnType == DataType::Undefined) {
    throw NodeEvaluationException{
      node, std::format("'{}' No matching overloaded function found.",
                        buildFunctionCall(data->name, transform(argTypes)))};
  }

  ShaderToken token{
    .name = nodeIdToString(node.vertex.id),
    .dataType = returnType,
  };
  m_composer.addVariable(token.dataType, token.name,
                         buildFunctionCall(data->name, extractArgNames(args)));
  m_tokens.emplace(std::move(token));
}
void ShaderCodeEvaluator::visit(const CustomNode &node) {
  const auto [functionId, data] = node.userFunction;
  if (!data) throw NodeEvaluationException{node, "Corrupted user function."};

  const auto numArguments = node.inputs.size();
  if (numArguments != data->inputs.size()) {
    throw NodeEvaluationException{node, "Invalid number of arguments."};
  }

  std::vector<ShaderToken> args;
  if (numArguments > 0) args = extractN(m_tokens, numArguments);
  auto const argTypes = extractArgTypes(args);

  if (!sameTypes(argTypes, data->inputs)) {
    throw NodeEvaluationException{
      node, std::format("'{}' No matching overloaded function found.",
                        buildFunctionCall(data->name, transform(argTypes)))};
  }

  ShaderToken token{
    .name = nodeIdToString(node.vertex),
    .dataType = data->output,
  };
  m_composer.addVariable(token.dataType, token.name,
                         buildFunctionCall(data->name, extractArgNames(args)));
  m_tokens.push(std::move(token));
  m_usedUserFunctions.emplace(functionId);
}

//
// Helper:
//

NodeEvaluationException::NodeEvaluationException(const NodeBase &node,
                                                 const std::string &message)
    : m_message{std::format("[node_id={}] {}", node.vertex.id, message)} {}

[[nodiscard]] const char *getPrefix(const rhi::ShaderType shaderType) {
  switch (shaderType) {
    using enum rhi::ShaderType;

  case Vertex:
    return "vs";
  case Fragment:
    return "fs";

  default:
    assert(false);
    return "_";
  }
}

std::string nodeIdToString(const VertexID id) {
  assert(id >= 0);
  return std::format("id_{}", std::to_string(id));
}

std::string buildFunctionCall(const std::string_view functionName,
                              const std::vector<std::string> &args) {
  return std::format("{}({})", functionName, join(args, ", "));
}

std::optional<std::string> assure(const ShaderToken &token,
                                  const DataType requiredType) {
  assert(requiredType != DataType::Undefined);
  if (token.isValid()) {
    if (token.dataType == requiredType) {
      return token.name;
    } else if (const auto fmt =
                 getConversionFormat(token.dataType, requiredType);
               fmt) {
      return std::vformat(*fmt, std::make_format_args(token.name));
    }
  }
  return std::nullopt;
}

std::vector<DataType> extractArgTypes(std::span<const ShaderToken> args) {
  std::vector<DataType> types(args.size());
  std::ranges::transform(args, types.begin(),
                         [](const auto &token) { return token.dataType; });
  return types;
}
std::vector<std::string> extractArgNames(std::span<const ShaderToken> args) {
  std::vector<std::string> names(args.size());
  std::ranges::transform(args, names.begin(),
                         [](const auto &token) { return token.name; });
  return names;
}

std::vector<std::string> transform(std::span<const DataType> in) {
  std::vector<std::string> out(in.size());
  std::ranges::transform(in, out.begin(),
                         [](const auto type) { return toString(type); });
  return out;
}

ShaderToken extractTop(std::stack<ShaderToken> &tokens) {
  assert(!tokens.empty());
  auto v = std::move(tokens.top());
  tokens.pop();
  return v;
}

std::vector<ShaderToken> extractN(std::stack<ShaderToken> &tokens,
                                  const std::size_t count) {
  std::vector<ShaderToken> args(count);
  extractN(tokens, count, args);
  return args;
}
