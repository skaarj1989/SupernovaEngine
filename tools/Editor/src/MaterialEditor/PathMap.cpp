#include "MaterialEditor/PathMap.hpp"
#include <cassert>

void PathMap::add(const rhi::ShaderType shaderType, const VertexID vertexId,
                  const std::filesystem::path &p) {
  const auto it = std::ranges::find(m_storage.paths, p);
  std::size_t idx;
  if (it != m_storage.paths.cend()) {
    idx = std::distance(m_storage.paths.begin(), it);
  } else {
    idx = m_storage.paths.size();
    m_storage.paths.emplace_back(p);
  }
  assert(idx >= 0);
  m_storage.ids[{shaderType, vertexId}] = idx;
}
bool PathMap::remove(const rhi::ShaderType shaderType,
                     const VertexID vertexId) {
  return m_storage.ids.erase({shaderType, vertexId}) > 0;
}

std::optional<std::filesystem::path>
PathMap::get(const rhi::ShaderType shaderType, const VertexID vertexId) const {
  if (const auto it = m_storage.ids.find({shaderType, vertexId});
      it != m_storage.ids.cend()) {
    return m_storage.paths[it->second];
  }
  return std::nullopt;
}

void PathMap::reset(Storage storage) { m_storage = std::move(storage); }

//
// Helper:
//

PathMap::Storage makeRelative(PathMap::Storage storage,
                              const std::filesystem::path &dir) {
  std::ranges::transform(
    storage.paths, storage.paths.begin(),
    [&dir](const auto &p) { return p.lexically_relative(dir); });
  return storage;
}
PathMap::Storage makeAbsolute(PathMap::Storage storage,
                              const std::filesystem::path &dir) {
  std::ranges::transform(
    storage.paths, storage.paths.begin(),
    [&dir](const auto &p) { return std::filesystem::absolute(dir / p); });
  return storage;
}
