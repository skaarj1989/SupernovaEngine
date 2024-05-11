#include "ShaderCodeEvaluator.hpp"
#include "Attribute.hpp"
#include "BuiltInConstants.hpp"
#include "FrameBlockMember.hpp"
#include "CameraBlockMember.hpp"
#include "Nodes/Embedded.hpp"

namespace {

[[nodiscard]] auto toGLSL(const Attribute attribute,
                          const gfx::MaterialDomain materialDomain) {
  switch (attribute) {
    using enum Attribute;
    using enum gfx::MaterialDomain;

  case Position:
    return "getPosition()";
  case Normal:
    return "getNormal()";
  case TexCoord0:
    return materialDomain == Surface ? "getTexCoord0()" : "v_TexCoord";
  case TexCoord1:
    if (materialDomain == Surface) return "getTexCoord1()";
    break;
  case Color:
    return "getColor()";

  default:
    break;
  }
  assert(false);
  return "";
}
[[nodiscard]] auto toGLSL(const BuiltInConstant e) {
  switch (e) {
    using enum BuiltInConstant;

  case CameraPosition:
    return "getCameraPosition()";
  case ScreenTexelSize:
    return "getScreenTexelSize()";
  case AspectRatio:
    return "getAspectRatio()";

  case ModelMatrix:
    return "getModelMatrix()";
  case FragPosWorldSpace:
    return "fragPos.worldSpace";
  case FragPosViewSpace:
    return "fragPos.viewSpace";

  case ViewDir:
    return "getViewDir()";

  default:
    assert(false);
    return "Undefined";
  }
}
[[nodiscard]] auto toGLSL(const BuiltInSampler e) {
  switch (e) {
    using enum BuiltInSampler;

  case SceneDepth:
    return "t_SceneDepth";
  case SceneColor:
    return "t_SceneColor";

  default:
    assert(false);
    return "Undefined";
  }
}

[[nodiscard]] auto toGLSL(const FrameBlockMember e) {
  switch (e) {
    using enum FrameBlockMember;

  case Time:
    return "time";
  case DeltaTime:
    return "deltaTime";

  default:
    assert(false);
    return "";
  }
}
[[nodiscard]] auto toGLSL(const CameraBlockMember e) {
  switch (e) {
    using enum CameraBlockMember;

  case Projection:
    return "projection";
  case InversedProjection:
    return "inversedProjection";
  case View:
    return "view";
  case InversedView:
    return "inversedView";
  case ViewProjection:
    return "viewProjection";
  case InversedViewProjection:
    return "inversedViewProjection";
  case Resolution:
    return "resolution";
  case Near:
    return "near";
  case Far:
    return "far";

  default:
    assert(false);
    return "";
  }
}

template <typename E, typename... Args>
[[nodiscard]] auto evaluateEnumNode(const EmbeddedNode<E> &node,
                                    Args &&...args) {
  return ShaderToken{
    .name = toGLSL(node.value, std::forward<Args>(args)...),
    .dataType = getDataType(node.value),
  };
}
template <typename E>
[[nodiscard]] auto evaluateBlockMember(const char *name,
                                       const EmbeddedNode<E> &node) {
  return ShaderToken({
    .name = std::format("{}.{}", name, toGLSL(node.value)),
    .dataType = getDataType(node.value),
  });
}

} // namespace

//
// ShaderCodeEvaluator class:
//

void ShaderCodeEvaluator::visit(const EmbeddedNode<Attribute> &node) {
  m_tokens.push(evaluateEnumNode(node, m_materialDomain));
}
void ShaderCodeEvaluator::visit(const EmbeddedNode<BuiltInConstant> &node) {
  m_tokens.push(evaluateEnumNode(node));
}
void ShaderCodeEvaluator::visit(const EmbeddedNode<BuiltInSampler> &node) {
  m_tokens.push(evaluateEnumNode(node));
}
void ShaderCodeEvaluator::visit(const EmbeddedNode<FrameBlockMember> &node) {
  m_tokens.push(evaluateBlockMember("u_Frame", node));
}
void ShaderCodeEvaluator::visit(const EmbeddedNode<CameraBlockMember> &node) {
  m_tokens.push(evaluateBlockMember("u_Camera", node));
}
