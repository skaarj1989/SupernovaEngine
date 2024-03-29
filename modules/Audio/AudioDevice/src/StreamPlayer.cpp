#include "audio/StreamPlayer.hpp"
#include "ALHelper.hpp"
#include "ALCheck.hpp"
#include <algorithm> // clamp

#include "tracy/Tracy.hpp"

namespace audio {

StreamPlayer::StreamPlayer(const int32_t numBuffers) {
  assert(numBuffers > 1);
  m_buffers.resize(numBuffers);
  AL_CHECK(alGenBuffers(numBuffers, m_buffers.data()));
}
StreamPlayer::~StreamPlayer() {
  stop();
  alSourcei(m_id, AL_BUFFER, AL_NONE);
  AL_CHECK(
    alDeleteBuffers(static_cast<ALsizei>(m_buffers.size()), m_buffers.data()));
}

void StreamPlayer::setStream(std::unique_ptr<Decoder> &&decoder,
                             const fsec bufferDuration) {
  if (decoder && decoder->isOpen()) {
    m_decoder = std::move(decoder);
  }
  if (!m_decoder) return;

  const auto &info = m_decoder->getInfo();
  m_format = pickFormat(info);
  m_stagingBuffer.resize(calcBufferSize(bufferDuration, info));
}
void StreamPlayer::setLooping(const bool b) { m_looping = b; }

fsec StreamPlayer::getDuration() const {
  return m_decoder ? m_decoder->getInfo().duration() : fsec{0};
}
bool StreamPlayer::isLooping() const { return m_looping; }

fsec StreamPlayer::tell() const {
  return m_decoder ? m_decoder->tell() : fsec{0};
}
void StreamPlayer::seek(const fsec offset) {
  if (m_decoder) {
    m_decoder->seek(std::clamp(offset, fsec{0},
                               m_decoder->getInfo().duration() -
                                 fsec{std::numeric_limits<float>::epsilon()}));
  }
}

void StreamPlayer::play() {
  if (!isPlaying()) _start();
  SourceBase::play();
}
void StreamPlayer::pause() { SourceBase::pause(); }
void StreamPlayer::stop() {
  SourceBase::stop();
  if (m_decoder) m_decoder->rewind();
}

void StreamPlayer::update() {
  using enum SourceBase::State;
  if (const auto state = getState(); state != Playing) return;

  ZoneScopedN("StreamPlayer::Update");

  ALint processed;
  AL_CHECK(alGetSourcei(m_id, AL_BUFFERS_PROCESSED, &processed));

  while (processed > 0) {
    ALuint bufferId{AL_NONE};
    alSourceUnqueueBuffers(m_id, 1, &bufferId);
    --processed;

    // Read the next chunk of data, refill the buffer,
    // and queue it back on the source.
    if (const auto decodedSize = _read(); decodedSize > 0) {
      AL_CHECK(alBufferData(bufferId, m_format, m_stagingBuffer.data(),
                            decodedSize, m_decoder->getInfo().sampleRate));
      AL_CHECK(alSourceQueueBuffers(m_id, 1, &bufferId));
    }

    if (m_looping && m_decoder->eos()) m_decoder->rewind();
  }

  if (!isPlaying()) {
    ALint queued;
    alGetSourcei(m_id, AL_BUFFERS_QUEUED, &queued);
    if (queued > 0) play();
  }
}

//
// (private):
//

void StreamPlayer::_start() {
  if (!m_decoder) return;

  alSourceRewind(m_id);
  alSourcei(m_id, AL_BUFFER, AL_NONE);

  int32_t numFilledBuffers{0};
  for (const auto bufferId : m_buffers) {
    const auto decodedSize = _read();
    if (decodedSize == 0) break;

    AL_CHECK(alBufferData(bufferId, m_format, m_stagingBuffer.data(),
                          decodedSize, m_decoder->getInfo().sampleRate));

    ++numFilledBuffers;
  }
  AL_CHECK(alSourceQueueBuffers(m_id, numFilledBuffers, m_buffers.data()));
}
uint32_t StreamPlayer::_read() {
  assert(m_decoder);
  const auto decodedSize =
    m_decoder->read(m_stagingBuffer.data(), m_stagingBuffer.size());
  return static_cast<uint32_t>(align(decodedSize, m_decoder->getInfo()));
}

} // namespace audio
