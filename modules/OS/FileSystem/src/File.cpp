#include "File.hpp"
#include <cassert>

namespace os {

PhysicalFile::PhysicalFile(const std::filesystem::path &p) {
#if _MSC_VER >= 1930
  const auto result = fopen_s(&m_file, p.string().c_str(), "rb");
  if (result == 0) _fetchSize();
#else
  m_file = fopen(p.string().c_str(), "rb");
  if (m_file) _fetchSize();
#endif
}
PhysicalFile::~PhysicalFile() { close(); }

int32_t PhysicalFile::close() {
  if (m_file) {
    const auto result = fclose(m_file);
    m_file = nullptr;
    return result;
  }
  return -1;
}

bool PhysicalFile::isOpen() const { return m_file != nullptr; }
std::size_t PhysicalFile::getSize() const { return m_size; }

std::size_t PhysicalFile::tell() const { return ftell(m_file); }
std::size_t PhysicalFile::seek(std::size_t offset, const Origin origin) {
  return fseek(m_file, offset, static_cast<int32_t>(origin));
}
std::size_t PhysicalFile::read(void *buffer, std::size_t length) {
  assert(length > 0);
  return fread(buffer, 1, length, m_file);
}

void PhysicalFile::_fetchSize() {
  seek(0, Origin::End);
  m_size = tell();
  rewind();
}

} // namespace os
