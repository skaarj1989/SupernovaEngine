#include "OzzFile.hpp"

namespace {

[[nodiscard]] auto convert(ozz::io::Stream::Origin origin) {
  switch (origin) {
    using enum ozz::io::Stream::Origin;
    using enum os::DataStream::Origin;

  case kSet:
    return Beginning;
  case kEnd:
    return End;
  case kCurrent:
  default:
    return Current;
  }
}

} // namespace

//
// OzzFileWrapper class:
//

OzzFileWrapper::OzzFileWrapper(std::unique_ptr<os::DataStream> &&stream)
    : m_stream{std::move(stream)} {}

bool OzzFileWrapper::opened() const { return m_stream && m_stream->isOpen(); }

size_t OzzFileWrapper::Read(void *buffer, size_t size) {
  return m_stream->read(buffer, size);
}
size_t OzzFileWrapper::Write(const void *, size_t) {
  assert(false); // Readonly.
  return 0;
}

int OzzFileWrapper::Seek(int offset, Origin origin) {
  return m_stream->seek(offset, convert(origin));
}
int OzzFileWrapper::Tell() const { return m_stream->tell(); }
size_t OzzFileWrapper::Size() const { return m_stream->getSize(); }
