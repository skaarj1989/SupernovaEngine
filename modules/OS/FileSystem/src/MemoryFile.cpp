#include "MemoryFile.hpp"
#include <cstring> // memcpy
#include <cassert>

namespace os {

MemoryFile::MemoryFile(char *data, std::size_t length)
    : m_filePtr{data}, m_currentPos{data}, m_size{length} {
  assert(data && length > 0);
}
MemoryFile::~MemoryFile() { close(); }

int32_t MemoryFile::close() {
  m_filePtr = nullptr;
  m_currentPos = nullptr;
  m_size = 0;
  return 0;
}

bool MemoryFile::isOpen() const { return m_filePtr != nullptr; }
std::size_t MemoryFile::getSize() const { return m_size; }

std::size_t MemoryFile::tell() const { return m_currentPos - m_filePtr; }
std::size_t MemoryFile::seek(std::size_t offset, Origin origin) {
  switch (origin) {
  case Origin::Current:
    m_currentPos += offset;
    break;
  case Origin::End:
    m_currentPos = m_filePtr + m_size - offset;
    break;
  case Origin::Beginning:
    m_currentPos = m_filePtr + offset;
    break;
  default:
    assert(false);
  }

  if (m_currentPos < m_filePtr) {
    m_currentPos = m_filePtr;
    return -1;
  }
  if (m_currentPos > m_filePtr + m_size) {
    m_currentPos = m_filePtr + m_size;
    return -1;
  }
  return 0;
}
std::size_t MemoryFile::read(void *buffer, std::size_t length) {
  assert(length > 0);
  if (m_currentPos + length > m_filePtr + m_size) {
    length = m_filePtr + m_size - m_currentPos;
  }
  std::memcpy(buffer, m_currentPos, length);
  m_currentPos += length;
  return length;
}

} // namespace os
