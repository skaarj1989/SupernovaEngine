#include "os/ZipArchive.hpp"
#include "InZipFile.hpp"

#include <ranges>

namespace os {

ZipArchive::ZipArchive(const std::filesystem::path &p) : m_path{p} {
  auto archive = unzOpen64(p.string().c_str());
  unz_global_info64 globalInfo;
  unzGetGlobalInfo64(archive, &globalInfo);
  constexpr auto kMaxFileNameLength = 256;
  do {
    const auto fileName = std::make_unique<char[]>(kMaxFileNameLength);
    unz_file_pos filePos;
    unzGetFilePos(archive, &filePos);
    unz_file_info64 fileInfo;
    unzGetCurrentFileInfo64(archive, &fileInfo, fileName.get(),
                            kMaxFileNameLength, nullptr, 0, nullptr, 0);
    if (fileInfo.compressed_size == 0) continue; // The "file" is a directory.

    EntryInfo entryInfo{
      .offset = filePos.pos_in_zip_directory,
      .uncompressedSize = fileInfo.uncompressed_size,
    };
    m_contents.try_emplace(fileName.get(), entryInfo);
  } while (unzGoToNextFile(archive) != UNZ_END_OF_LIST_OF_FILE);
  unzClose(archive);
}

std::vector<std::string> ZipArchive::statContent() const {
  std::vector<std::string> keys;
  keys.reserve(m_contents.size());
  std::ranges::copy(m_contents | std::views::keys, std::back_inserter(keys));
  return keys;
}

bool ZipArchive::hasFile(const std::string_view path) const {
  return m_contents.contains(path);
}

std::unique_ptr<DataStream> ZipArchive::mapFile(const std::string_view path) {
  const auto it = m_contents.find(path.data());
  if (it == m_contents.cend()) return {};

  const auto &[offset, size] = it->second;

  auto archive = unzOpen64(m_path.string().c_str());
  unzSetOffset(archive, offset); // or unzGoToFilePos64(m_archive, offset) ?
  unzOpenCurrentFile(archive);
  return std::make_unique<InZipFile>(archive, offset, size);
}

} // namespace os
