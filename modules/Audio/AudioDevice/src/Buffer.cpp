#include "audio/Buffer.hpp"
#include "ALHelper.hpp"
#include "ALCheck.hpp"

namespace audio {

Buffer::Buffer(Buffer &&other) noexcept : m_id{other.m_id} {
  other.m_id = AL_NONE;
  std::swap(m_info, other.m_info);
}
Buffer::~Buffer() { _destroy(); }

Buffer &Buffer::operator=(Buffer &&rhs) noexcept {
  if (this != &rhs) {
    _destroy();
    std::swap(m_id, rhs.m_id);
    std::swap(m_info, rhs.m_info);
  }
  return *this;
}

const ClipInfo &Buffer::getInfo() const { return m_info; }

//
// (private):
//

Buffer::Buffer(const ClipInfo &info, const void *data) : m_info{info} {
  alGenBuffers(1, &m_id);
  AL_CHECK(alBufferData(m_id, pickFormat(info), data, info.dataSize(),
                        info.sampleRate));
}

void Buffer::_destroy() {
  if (m_id != AL_NONE) {
    AL_CHECK(alDeleteBuffers(1, &m_id));
    m_id = AL_NONE;
  }
}

} // namespace audio
