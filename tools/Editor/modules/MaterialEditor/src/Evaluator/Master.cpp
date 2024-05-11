#include "ShaderCodeEvaluator.hpp"
#include "Nodes/VertexMaster.hpp"
#include "Nodes/SurfaceMaster.hpp"
#include "Nodes/PostProcessMaster.hpp"

#include <format>
#include <ranges>

void ShaderCodeEvaluator::visit(const VertexMasterNode &node) {
  if (m_tokens.size() != VertexMasterNode::kFields.size()) {
    throw NodeEvaluationException{node, "Unexpected number of inputs."};
  }

  for (const auto [i, arg] :
       extractN(m_tokens, VertexMasterNode::kFields.size()) |
         std::views::enumerate) {
    const auto &[name, defaultValue] = VertexMasterNode::kFields[i];
    const auto requiredType = getDataType(defaultValue);
    auto argStr = assure(arg, requiredType);
    if (!argStr) {
      throw NodeEvaluationException{
        node, std::format("'{}': Can't convert {} -> {}.", name,
                          toString(arg.dataType), toString(requiredType))};
    }
    m_composer.addExpression(std::format("{} = {};", name, *argStr));
  }
}
void ShaderCodeEvaluator::visit(const SurfaceMasterNode &node) {
  if (m_tokens.size() != SurfaceMasterNode::kFields.size()) {
    throw NodeEvaluationException{node, "Unexpected number of inputs."};
  }

  for (const auto [i, arg] :
       extractN(m_tokens, SurfaceMasterNode::kFields.size()) |
         std::views::enumerate) {
    const auto &[name, defaultValue, _] = SurfaceMasterNode::kFields[i];
    const auto requiredType = getDataType(defaultValue);
    auto argStr = assure(arg, requiredType);
    if (!argStr) {
      throw NodeEvaluationException{
        node, std::format("'{}': Can't convert {} -> {}.", name,
                          toString(arg.dataType), toString(requiredType))};
    }
    m_composer.addExpression(std::format("material.{} = {};", name, *argStr));
  }
}
void ShaderCodeEvaluator::visit(const PostProcessMasterNode &node) {
  if (m_tokens.size() != PostProcessMasterNode::kFields.size()) {
    throw NodeEvaluationException{node, "Unexpected number of inputs."};
  }

  for (const auto [i, arg] :
       extractN(m_tokens, PostProcessMasterNode::kFields.size()) |
         std::views::enumerate) {
    const auto &[name, defaultValue] = PostProcessMasterNode::kFields[i];
    const auto requiredType = getDataType(defaultValue);
    auto argStr = assure(arg, requiredType);
    if (!argStr) {
      throw NodeEvaluationException{
        node, std::format("'{}': Can't convert {} -> {}.", name,
                          toString(arg.dataType), toString(requiredType))};
    }
    m_composer.addExpression(std::format("{} = {};", name, *argStr));
  }
}
