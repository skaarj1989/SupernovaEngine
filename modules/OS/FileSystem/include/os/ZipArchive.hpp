#pragma once

#include "os/DataStream.hpp"
#include "StringHash.hpp"
#include <filesystem>
#include <memory>
#include <vector>
#include <unordered_map>

namespace os {

class ZipArchive final {
public:
  ZipArchive() = default;
  explicit ZipArchive(const std::filesystem::path &);
  ~ZipArchive() = default;

  [[nodiscard]] std::vector<std::string> statContent() const;
  [[nodiscard]] bool hasFile(const std::string_view path) const;
  [[nodiscard]] std::unique_ptr<DataStream>
  mapFile(const std::string_view path);

private:
  std::filesystem::path m_path;
  struct EntryInfo {
    uint64_t offset{0}; // Position inside an archive.
    uint64_t uncompressedSize{0};
  };
  std::unordered_map<std::string, EntryInfo, StringHash, std::equal_to<>>
    m_contents;
};

} // namespace os
