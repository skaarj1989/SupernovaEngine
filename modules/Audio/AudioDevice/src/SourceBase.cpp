#include "audio/SourceBase.hpp"
#include "ALCheck.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <utility>

namespace audio {

SourceBase::SourceBase(SourceBase &&other) noexcept : m_id{other.m_id} {
  other.m_id = AL_NONE;
}
SourceBase::~SourceBase() { _destroy(); }

SourceBase &SourceBase::operator=(SourceBase &&rhs) noexcept {
  if (this != &rhs) {
    _destroy();
    std::swap(m_id, rhs.m_id);
  }
  return *this;
}

void SourceBase::play() { AL_CHECK(alSourcePlay(m_id)); }
void SourceBase::pause() { AL_CHECK(alSourcePause(m_id)); }
void SourceBase::stop() { AL_CHECK(alSourceStop(m_id)); }

void SourceBase::setPosition(const glm::vec3 &v) {
  AL_CHECK(alSourcefv(m_id, AL_POSITION, glm::value_ptr(v)));
}
void SourceBase::setDirection(const glm::vec3 &v) {
  AL_CHECK(alSourcefv(m_id, AL_DIRECTION, glm::value_ptr(v)));
}
void SourceBase::setVelocity(const glm::vec3 &v) {
  AL_CHECK(alSourcefv(m_id, AL_VELOCITY, glm::value_ptr(v)));
}

void SourceBase::setPitch(const float s) {
  AL_CHECK(alSourcef(m_id, AL_PITCH, glm::max(0.0f, s)));
}
void SourceBase::setGain(const float s) {
  AL_CHECK(alSourcef(m_id, AL_GAIN, glm::max(0.0f, s)));
}
void SourceBase::setMaxDistance(const float s) {
  AL_CHECK(alSourcef(m_id, AL_MAX_DISTANCE, s));
}
void SourceBase::setRollOffFactor(const float s) {
  AL_CHECK(alSourcef(m_id, AL_ROLLOFF_FACTOR, s));
}
void SourceBase::setReferenceDistance(const float s) {
  AL_CHECK(alSourcef(m_id, AL_REFERENCE_DISTANCE, s));
}
void SourceBase::setMinGain(const float s) {
  AL_CHECK(alSourcef(m_id, AL_MIN_GAIN, s));
}
void SourceBase::setMaxGain(const float s) {
  AL_CHECK(alSourcef(m_id, AL_MAX_GAIN, s));
}

float SourceBase::getPitch() const {
  ALfloat s;
  AL_CHECK(alGetSourcef(m_id, AL_PITCH, &s));
  return s;
}
float SourceBase::getGain() const {
  ALfloat s;
  AL_CHECK(alGetSourcef(m_id, AL_GAIN, &s));
  return s;
}
float SourceBase::getMaxDistance() const {
  ALfloat s;
  AL_CHECK(alGetSourcef(m_id, AL_MAX_DISTANCE, &s));
  return s;
}
float SourceBase::getRollOffFactor() const {
  ALfloat s;
  AL_CHECK(alGetSourcef(m_id, AL_ROLLOFF_FACTOR, &s));
  return s;
}
float SourceBase::getReferenceDistance() const {
  ALfloat s;
  AL_CHECK(alGetSourcef(m_id, AL_REFERENCE_DISTANCE, &s));
  return s;
}
float SourceBase::getMinGain() const {
  ALfloat s;
  AL_CHECK(alGetSourcef(m_id, AL_MIN_GAIN, &s));
  return s;
}
float SourceBase::getMaxGain() const {
  ALfloat s;
  AL_CHECK(alGetSourcef(m_id, AL_MAX_GAIN, &s));
  return s;
}

glm::vec3 SourceBase::getPosition() const {
  glm::vec3 v;
  alGetSourcefv(m_id, AL_POSITION, glm::value_ptr(v));
  return v;
}
glm::vec3 SourceBase::getDirection() const {
  glm::vec3 v;
  alGetSourcefv(m_id, AL_DIRECTION, glm::value_ptr(v));
  return v;
}
glm::vec3 SourceBase::getVelocity() const {
  glm::vec3 v;
  alGetSourcefv(m_id, AL_VELOCITY, glm::value_ptr(v));
  return v;
}

SourceBase::State SourceBase::getState() const {
  ALint state;
  AL_CHECK(alGetSourcei(m_id, AL_SOURCE_STATE, &state));

  using enum State;
  switch (state) {
  case AL_PLAYING:
    return Playing;
  case AL_PAUSED:
    return Paused;
  case AL_STOPPED:
    return Stopped;
  }
  return Initial;
}

bool SourceBase::isPlaying() const { return getState() == State::Playing; }
bool SourceBase::isPaused() const { return getState() == State::Paused; }
bool SourceBase::isStopped() const { return getState() == State::Stopped; }

//
// (private):
//

SourceBase::SourceBase() { alGenSources(1, &m_id); }

void SourceBase::_destroy() {
  if (m_id != AL_NONE) {
    AL_CHECK(alDeleteSources(1, &m_id));
    m_id = AL_NONE;
  }
}

} // namespace audio
