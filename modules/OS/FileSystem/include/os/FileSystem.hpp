#pragma once

#include "DataStream.hpp"
#include <filesystem>
#include <expected>

namespace os {

struct RawBuffer {
  std::unique_ptr<std::byte[]> data;
  std::size_t size;
};

class FileSystem final {
public:
  FileSystem() = delete;

  static std::vector<std::filesystem::path> getVolumes();

  static void setRoot(std::filesystem::path);
  static const std::filesystem::path &getRoot();

  static std::string_view getForbiddenCharacters();

  // Always lowercase (with leading dot character).
  [[nodiscard]] static std::optional<std::string>
  getExtension(const std::filesystem::path &);

  static std::optional<std::filesystem::path>
  relativeToRoot(const std::filesystem::path &);

  // --

  static bool saveText(const std::filesystem::path &, const std::string &);

  [[nodiscard]] static std::unique_ptr<DataStream>
  mapFile(const std::filesystem::path &);

  [[nodiscard]] static std::unique_ptr<DataStream> mapMemory(char *data,
                                                             size_t length);

  [[nodiscard]] static std::string readText(DataStream &);
  // @see mapFile
  [[nodiscard]] static std::expected<std::string, std::string>
  readText(const std::filesystem::path &);
  [[nodiscard]] static RawBuffer readBuffer(DataStream &);
  // @see mapFile
  [[nodiscard]] static std::expected<RawBuffer, std::string>
  readBuffer(const std::filesystem::path &);

private:
  static std::filesystem::path m_rootPath;
};

} // namespace os
