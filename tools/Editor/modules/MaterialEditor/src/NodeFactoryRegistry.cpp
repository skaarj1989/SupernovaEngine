#include "NodeFactoryRegistry.hpp"

#include "TextureParam.hpp"
#include "Attribute.hpp"
#include "BuiltInConstants.hpp"
#include "FrameBlockMember.hpp"
#include "CameraBlockMember.hpp"
#include "Nodes/Embedded.hpp"

#include "Nodes/Append.hpp"
#include "Nodes/VectorSplitter.hpp"
#include "Nodes/MatrixSplitter.hpp"
#include "Nodes/Swizzle.hpp"
#include "Nodes/Arithmetic.hpp"
#include "Nodes/MatrixTransform.hpp"
#include "Nodes/TextureSampling.hpp"
#include "Nodes/Scripted.hpp"
#include "Nodes/Custom.hpp"

#include "ShaderGraph.hpp"

namespace {

template <class T> [[nodiscard]] auto createEmbeddedNodeFactory(T &&value) {
  return [value](ShaderGraph &g, auto hint) {
    return g.add<EmbeddedNode<T>>(hint, value)->vertex;
  };
}

template <typename T> [[nodiscard]] auto makeConstantFactoryPair(T &&value) {
  return std::pair{
    hashConstant<T>(),
    createEmbeddedNodeFactory(std::forward<ValueVariant>(value)),
  };
}
template <typename T> [[nodiscard]] auto makePropertyFactoryPair(T &&value) {
  return std::pair{
    hashProperty<T>(),
    createEmbeddedNodeFactory(std::forward<PropertyVariant>(value)),
  };
}
template <typename E> [[nodiscard]] auto makeEnumFactoryPair(E &&value) {
  return std::pair{
    hashEnum(value),
    createEmbeddedNodeFactory(std::forward<E>(value)),
  };
}
template <class NodeT> [[nodiscard]] auto makeCompoundNodeFactoryPair() {
  return std::pair{
    typeid(NodeT).hash_code(),
    [](ShaderGraph &g, auto hint) { return g.add<NodeT>(hint)->vertex; },
  };
}
[[nodiscard]] auto
makeArithmeticFactoryPair(const ArithmeticNode::Operation op) {
  return std::pair{
    hashEnum(op),
    [op](ShaderGraph &g, auto hint) {
      return g.add<ArithmeticNode>(hint, op)->vertex;
    },
  };
}

template <typename NodeT, typename Handle>
bool addFactory(auto &storage, Handle handle) {
  const auto [_, inserted] =
    storage.try_emplace(handle.id, [handle](ShaderGraph &g, auto hint) {
      return g.add<NodeT>(hint, handle)->vertex;
    });
  return inserted;
}

} // namespace

NodeFactoryRegistry::NodeFactoryRegistry() {
  m_storage = {
    makeEnumFactoryPair(Attribute::Position),
    makeEnumFactoryPair(Attribute::TexCoord0),
    makeEnumFactoryPair(Attribute::TexCoord1),
    makeEnumFactoryPair(Attribute::Normal),
    makeEnumFactoryPair(Attribute::Color),

    makeEnumFactoryPair(BuiltInConstant::CameraPosition),
    makeEnumFactoryPair(BuiltInConstant::ScreenTexelSize),
    makeEnumFactoryPair(BuiltInConstant::AspectRatio),
    makeEnumFactoryPair(BuiltInConstant::ModelMatrix),
    makeEnumFactoryPair(BuiltInConstant::FragPosWorldSpace),
    makeEnumFactoryPair(BuiltInConstant::FragPosViewSpace),
    makeEnumFactoryPair(BuiltInConstant::ViewDir),

    makeEnumFactoryPair(BuiltInSampler::SceneDepth),
    makeEnumFactoryPair(BuiltInSampler::SceneColor),

    makeEnumFactoryPair(FrameBlockMember::Time),
    makeEnumFactoryPair(FrameBlockMember::DeltaTime),

    makeEnumFactoryPair(CameraBlockMember::Projection),
    makeEnumFactoryPair(CameraBlockMember::InversedProjection),
    makeEnumFactoryPair(CameraBlockMember::View),
    makeEnumFactoryPair(CameraBlockMember::InversedView),
    makeEnumFactoryPair(CameraBlockMember::ViewProjection),
    makeEnumFactoryPair(CameraBlockMember::InversedViewProjection),
    makeEnumFactoryPair(CameraBlockMember::Resolution),
    makeEnumFactoryPair(CameraBlockMember::Near),
    makeEnumFactoryPair(CameraBlockMember::Far),

    makeConstantFactoryPair(false),
    makeConstantFactoryPair(glm::bvec2{false}),
    makeConstantFactoryPair(glm::bvec3{false}),
    makeConstantFactoryPair(glm::bvec4{false}),

    makeConstantFactoryPair(int32_t{0}),
    makeConstantFactoryPair(glm::ivec2{0}),
    makeConstantFactoryPair(glm::ivec3{0}),
    makeConstantFactoryPair(glm::ivec4{0}),

    makeConstantFactoryPair(uint32_t{0}),
    makeConstantFactoryPair(glm::uvec2{0}),
    makeConstantFactoryPair(glm::uvec3{0}),
    makeConstantFactoryPair(glm::uvec4{0}),

    makeConstantFactoryPair(0.0f),
    makeConstantFactoryPair(glm::vec2{0.0f}),
    makeConstantFactoryPair(glm::vec3{0.0f}),
    makeConstantFactoryPair(glm::vec4{0.0f}),

    makeConstantFactoryPair(0.0),
    makeConstantFactoryPair(glm::dvec2{0.0}),
    makeConstantFactoryPair(glm::dvec3{0.0}),
    makeConstantFactoryPair(glm::dvec4{0.0}),

    makeConstantFactoryPair(glm::mat2{1.0f}),
    makeConstantFactoryPair(glm::mat3{1.0f}),
    makeConstantFactoryPair(glm::mat4{1.0f}),

    makePropertyFactoryPair(0),
    makePropertyFactoryPair(0u),
    makePropertyFactoryPair(0.0f),
    makePropertyFactoryPair(glm::vec2{0.0f}),
    makePropertyFactoryPair(glm::vec4{0.0f}),

    std::pair{
      typeid(TextureParam).hash_code(),
      createEmbeddedNodeFactory(TextureParam{}),
    },

    makeArithmeticFactoryPair(ArithmeticNode::Operation::Add),
    makeArithmeticFactoryPair(ArithmeticNode::Operation::Subtract),
    makeArithmeticFactoryPair(ArithmeticNode::Operation::Multiply),
    makeArithmeticFactoryPair(ArithmeticNode::Operation::Divide),

    makeCompoundNodeFactoryPair<AppendNode>(),
    makeCompoundNodeFactoryPair<VectorSplitterNode>(),
    makeCompoundNodeFactoryPair<MatrixSplitterNode>(),
    makeCompoundNodeFactoryPair<SwizzleNode>(),
    makeCompoundNodeFactoryPair<MatrixTransformNode>(),
    makeCompoundNodeFactoryPair<TextureSamplingNode>(),
  };
}

bool NodeFactoryRegistry::add(const ScriptedFunction::Handle scriptedFunction) {
  return addFactory<ScriptedNode>(m_storage, scriptedFunction);
}
bool NodeFactoryRegistry::add(const UserFunction::Handle userFunction) {
  return addFactory<CustomNode>(m_storage, userFunction);
}
bool NodeFactoryRegistry::remove(const FactoryID factoryId) {
  return m_storage.erase(factoryId) > 0;
}

IDPair NodeFactoryRegistry::create(const FactoryID factoryId, ShaderGraph &g,
                                   std::optional<VertexID> hint) const {
  const auto it = m_storage.find(factoryId);
  return it != m_storage.cend() ? it->second(g, hint) : IDPair{};
}
