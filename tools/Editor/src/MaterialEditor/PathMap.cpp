#include "MaterialEditor/PathMap.hpp"
#include <cassert>

PathMap::PathMap(const std::filesystem::path &root) : m_root{root} {}

void PathMap::addPath(const rhi::ShaderType shaderType, int32_t nodeId,
                      const std::filesystem::path &p) {
  auto relativePath = p.lexically_relative(m_root).generic_string();
  const auto it = std::ranges::find(m_relativePaths, relativePath);
  const auto idx = it == m_relativePaths.cend()
                     ? _emplace(std::move(relativePath))
                     : std::distance(m_relativePaths.begin(), it);
  assert(idx >= 0);
  m_ids[{shaderType, nodeId}] = idx;
}
std::optional<std::filesystem::path>
PathMap::getPath(const rhi::ShaderType shaderType, int32_t nodeId) const {
  if (const auto it = m_ids.find({shaderType, nodeId}); it != m_ids.cend()) {
    return m_root / m_relativePaths[it->second];
  }
  return std::nullopt;
}

std::size_t PathMap::_emplace(std::string &&str) {
  const auto id = m_relativePaths.size();
  m_relativePaths.emplace_back(std::move(str));
  return id;
}
