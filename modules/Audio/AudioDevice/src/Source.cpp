#include "audio/Source.hpp"
#include "ALCheck.hpp"

namespace audio {

Source::Source(std::shared_ptr<Buffer> buffer) { setBuffer(std::move(buffer)); }
Source::~Source() { setBuffer(nullptr); }

void Source::setBuffer(std::shared_ptr<Buffer> buffer) {
  stop();
  AL_CHECK(alSourcei(m_id, AL_BUFFER, buffer ? buffer->m_id : AL_NONE));
  m_buffer = std::move(buffer);
}
void Source::setLooping(const bool b) {
  AL_CHECK(alSourcei(m_id, AL_LOOPING, b));
}

std::shared_ptr<Buffer> Source::getBuffer() const { return m_buffer; }
bool Source::isLooping() const {
  ALint looping;
  AL_CHECK(alGetSourcei(m_id, AL_LOOPING, &looping));
  return looping;
}

fsec Source::tell() const {
  ALfloat offset;
  AL_CHECK(alGetSourcef(m_id, AL_SEC_OFFSET, &offset));
  return fsec{offset};
}
void Source::seek(const fsec offset) {
  AL_CHECK(alSourcef(m_id, AL_SEC_OFFSET, offset.count()));
}

} // namespace audio
