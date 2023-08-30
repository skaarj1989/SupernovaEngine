#include "os/FileSystem.hpp"
#include "File.hpp"
#include "MemoryFile.hpp"

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <Windows.h>
#endif
#include <fstream>

namespace os {

namespace {

void validate(const DataStream &stream) {
  if (!stream.isOpen()) {
    throw std::runtime_error{"Stream is not opened."};
  } else if (stream.getSize() == 0) {
    throw std::runtime_error{"Empty file."};
  }
}

} // namespace

//
// FileSystem class:
//

std::filesystem::path FileSystem::m_rootPath{std::filesystem::current_path()};

std::vector<std::filesystem::path> FileSystem::getVolumes() {
  std::vector<std::filesystem::path> volumes;

#ifdef _WIN32
  constexpr auto kMaxNumDrives = 26;
  volumes.reserve(kMaxNumDrives);

  const auto logicalDrivesMask = ::GetLogicalDrives();
  for (auto i = 0; i < kMaxNumDrives; ++i) {
    if (logicalDrivesMask & (1 << i)) {
      const auto letter = static_cast<char>('a' + i);
      std::filesystem::path mnt = std::format(R"({}:\)", letter);
      std::error_code ec;
      if (std::filesystem::exists(mnt, ec)) {
        volumes.emplace_back(std::move(mnt));
      }
    }
  }
#endif
  return volumes;
}

void FileSystem::setRoot(std::filesystem::path p) { m_rootPath = std::move(p); }
const std::filesystem::path &FileSystem::getRoot() { return m_rootPath; }

std::string_view FileSystem::getForbiddenCharacters() { return R"(\/:*?"<>|)"; }

std::optional<std::string>
FileSystem::getExtension(const std::filesystem::path &p) {
  auto ext = p.extension().string();
  if (ext.empty()) return std::nullopt;
  std::ranges::transform(ext, ext.begin(), ::tolower);
  return ext;
}

std::optional<std::filesystem::path>
FileSystem::relativeToRoot(const std::filesystem::path &absolute) {
  std::optional p = absolute.lexically_relative(m_rootPath);
  if (p->empty()) p.reset();
  return p;
}

bool FileSystem::saveText(const std::filesystem::path &p,
                          const std::string &s) {
  if (auto f = std::ofstream{p, std::ios::binary | std::ios::trunc};
      f.is_open()) {
    f << s;
    return true;
  }
  return false;
}

std::unique_ptr<DataStream>
FileSystem::mapFile(const std::filesystem::path &p) {
  auto stream = std::make_unique<PhysicalFile>(p);
  if (!stream->isOpen()) stream.reset();
  return stream;
}

std::unique_ptr<DataStream> FileSystem::mapMemory(char *data, size_t length) {
  return std::make_unique<MemoryFile>(data, length);
}

std::expected<std::string, std::string>
FileSystem::readText(const std::filesystem::path &p) {
  auto stream = mapFile(p);
  if (!stream) {
    return std::unexpected{"Could not open file."};
  }
  try {
    return readText(*stream);
  } catch (const std::exception &e) {
    return std::unexpected{e.what()};
  }
}

std::string FileSystem::readText(DataStream &stream) {
  validate(stream);
  const auto length = stream.getSize();
  std::string text;
  text.resize(length);
  stream.read(text.data(), length);
  return text;
}

RawBuffer FileSystem::readBuffer(DataStream &stream) {
  validate(stream);
  RawBuffer buffer{.size = stream.getSize()};
  buffer.data = std::make_unique<std::byte[]>(buffer.size);
  stream.read(buffer.data.get(), buffer.size);
  return buffer;
}
std::expected<RawBuffer, std::string>
FileSystem::readBuffer(const std::filesystem::path &p) {
  auto stream = mapFile(p);
  if (!stream) {
    return std::unexpected{"Could not open file."};
  }
  try {
    return readBuffer(*stream);
  } catch (const std::exception &e) {
    return std::unexpected{e.what()};
  }
}

} // namespace os
