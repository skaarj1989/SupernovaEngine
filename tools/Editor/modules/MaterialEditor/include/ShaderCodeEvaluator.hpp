#pragma once

#include "Nodes/NodeVisitor.hpp"

#include "ShaderGraphCommon.hpp"
#include "ShaderToken.hpp"
#include "ShaderCodeComposer.hpp"
#include "renderer/Material.hpp"

#include "UserFunction.hpp"
#include "TransientVariant.hpp"

#include <array>
#include <stack>
#include <expected>

class ShaderGraph;

struct ShaderDefinition {
  std::string code;

  template <typename T> using Map = std::map<std::string, T, std::less<>>;
  using PropertyMap = Map<gfx::Property::Value>;
  PropertyMap properties;
  using TextureMap = Map<std::shared_ptr<rhi::Texture>>;
  TextureMap textures;

  std::vector<UserFunction::ID> usedUserFunctions;
};

class ShaderCodeEvaluator : private NodeVisitor {
public:
  explicit ShaderCodeEvaluator(const gfx::MaterialDomain materialDomain)
      : m_materialDomain{materialDomain} {}

  using ErrorMessage = std::string;
  using Result = std::expected<ShaderDefinition, ErrorMessage>;
  [[nodiscard]] Result evaluate(const ShaderGraph &);

private:
  void _clear();

  void visit(const EmptyNode &) override;

  void visit(const EmbeddedNode<ValueVariant> &) override;
  void visit(const EmbeddedNode<PropertyVariant> &) override;
  void visit(const EmbeddedNode<TextureParam> &) override;

  void visit(const EmbeddedNode<Attribute> &) override;
  void visit(const EmbeddedNode<BuiltInConstant> &) override;
  void visit(const EmbeddedNode<BuiltInSampler> &) override;
  void visit(const EmbeddedNode<FrameBlockMember> &) override;
  void visit(const EmbeddedNode<CameraBlockMember> &) override;

  void visit(const EmbeddedNode<SplitVector> &) override;
  void visit(const EmbeddedNode<SplitMatrix> &) override;

  void visit(const AppendNode &) override;
  void visit(const VectorSplitterNode &) override;
  void visit(const MatrixSplitterNode &) override;
  void visit(const SwizzleNode &) override;
  void visit(const ArithmeticNode &) override;
  void visit(const MatrixTransformNode &) override;
  void visit(const TextureSamplingNode &) override;
  void visit(const ScriptedNode &) override;
  void visit(const CustomNode &) override;

  void visit(const VertexMasterNode &) override;
  void visit(const SurfaceMasterNode &) override;
  void visit(const PostProcessMasterNode &) override;

private:
  gfx::MaterialDomain m_materialDomain;

  std::stack<ShaderToken> m_tokens;
  ShaderCodeComposer m_composer;
  ShaderDefinition::PropertyMap m_properties;
  ShaderDefinition::TextureMap m_textures;
  std::unordered_set<UserFunction::ID> m_usedUserFunctions;
};

//
// Helper:
//

class NodeEvaluationException : public std::exception {
public:
  NodeEvaluationException(const NodeBase &, const std::string &message);

  const char *what() const noexcept override { return m_message.c_str(); }

private:
  std::string m_message;
};

[[nodiscard]] const char *getPrefix(const rhi::ShaderType);

[[nodiscard]] std::string nodeIdToString(const VertexID);

[[nodiscard]] std::string
buildFunctionCall(const std::string_view functionName,
                  const std::vector<std::string> &args);

std::optional<std::string> assure(const ShaderToken &,
                                  const DataType requiredType);

template <std::size_t _Size>
[[nodiscard]] auto extract(std::stack<ShaderToken> &tokens) {
  static_assert(_Size > 0);
  std::array<ShaderToken, _Size> arr;
  extractN(tokens, _Size, arr);
  return arr;
}

[[nodiscard]] std::vector<DataType>
  extractArgTypes(std::span<const ShaderToken>);
[[nodiscard]] std::vector<std::string>
  extractArgNames(std::span<const ShaderToken>);

[[nodiscard]] std::vector<std::string> transform(std::span<const DataType>);

ShaderToken extractTop(std::stack<ShaderToken> &);

inline void extractN(std::stack<ShaderToken> &tokens, const std::size_t count,
                     auto &out) {
  assert(count > 0 && tokens.size() >= count);
  std::generate_n(out.rbegin(), count,
                  [&tokens] { return extractTop(tokens); });
}
[[nodiscard]] std::vector<ShaderToken> extractN(std::stack<ShaderToken> &,
                                                const std::size_t count);
