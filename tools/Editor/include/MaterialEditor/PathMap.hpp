#pragma once

#include "rhi/ShaderType.hpp"
#include <filesystem>
#include <optional>
#include <map>
#include <vector>

class PathMap {
public:
  explicit PathMap(const std::filesystem::path &);

  void addPath(const rhi::ShaderType, int32_t nodeId,
               const std::filesystem::path &);
  [[nodiscard]] std::optional<std::filesystem::path>
  getPath(const rhi::ShaderType, int32_t nodeId) const;

  template <class Archive> void serialize(Archive &archive) {
    archive(m_relativePaths, m_ids);
  }

private:
  [[nodiscard]] std::size_t _emplace(std::string &&);

private:
  std::filesystem::path m_root;
  std::vector<std::string> m_relativePaths;

  // .second = nodeId
  using Key = std::pair<rhi::ShaderType, int32_t>;
  std::map<Key, std::size_t> m_ids;
};
