#pragma once

#include "rhi/ShaderType.hpp"
#include "ShaderGraphCommon.hpp"
#include <map>
#include <vector>
#include <optional>
#include <filesystem>

class PathMap {
public:
  struct Storage {
    std::vector<std::filesystem::path> paths;
    std::map<std::pair<rhi::ShaderType, VertexID>, std::size_t> ids;

    template <class Archive> void serialize(Archive &archive) {
      archive(paths, ids);
    }
  };

  void add(const rhi::ShaderType, const VertexID,
           const std::filesystem::path &);
  bool remove(const rhi::ShaderType, const VertexID);

  [[nodiscard]] std::optional<std::filesystem::path> get(const rhi::ShaderType,
                                                         const VertexID) const;

  void reset(Storage = {});

  const Storage &getStorage() const { return m_storage; }

private:
  Storage m_storage;
};

PathMap::Storage makeRelative(PathMap::Storage, const std::filesystem::path &);
PathMap::Storage makeAbsolute(PathMap::Storage, const std::filesystem::path &);
