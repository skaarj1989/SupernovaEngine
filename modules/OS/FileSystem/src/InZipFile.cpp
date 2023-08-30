#include "InZipFile.hpp"
#include <memory>
#include <cassert>

namespace os {

InZipFile::InZipFile(unzFile archive, uint64_t offset, uint64_t size)
    : m_archive{archive}, m_startOffset{offset}, m_uncompressedSize{size} {
  assert(size > 0);
}
InZipFile::~InZipFile() { close(); }

int32_t InZipFile::close() {
  if (isOpen()) {
    unzClose(m_archive);
    m_archive = nullptr;
    m_startOffset = 0;
    m_uncompressedSize = 0;
  }
  return 0;
}

bool InZipFile::isOpen() const { return m_archive != nullptr; }
std::size_t InZipFile::getSize() const {
  assert(isOpen());
  return m_uncompressedSize;
}

std::size_t InZipFile::tell() const {
  assert(isOpen());
  return unztell(m_archive);
}

std::size_t InZipFile::seek(std::size_t offset, const Origin origin) {
  assert(isOpen());

  auto i = 0u;
  auto bytesRead = 0u;
  switch (origin) {
  case Origin::End:
    offset = m_uncompressedSize - offset;
    [[fallthrough]];
  case Origin::Beginning:
    unzSetOffset64(m_archive, m_startOffset);
    unzOpenCurrentFile(m_archive);
    if (offset <= 0) return 0;
    [[fallthrough]];
  case Origin::Current:
    constexpr auto kSeekBufferSize = (1 << 15);
    auto buffer = std::make_unique<char[]>(kSeekBufferSize);
    for (i = 0; i < (offset - kSeekBufferSize); i += kSeekBufferSize) {
      bytesRead = unzReadCurrentFile(m_archive, buffer.get(), kSeekBufferSize);
      assert(bytesRead == kSeekBufferSize);
      if (bytesRead < kSeekBufferSize) return -1;
    }
    bytesRead = i + unzReadCurrentFile(m_archive, buffer.get(), offset - i);
    assert(bytesRead == offset);
    return bytesRead == offset ? 0 : -1;
  }
  assert(false);
  return -1;
}
std::size_t InZipFile::read(void *buffer, std::size_t length) {
  assert(isOpen() && length > 0);
  return unzReadCurrentFile(m_archive, buffer, length);
}

} // namespace os
