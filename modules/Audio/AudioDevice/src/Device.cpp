#include "audio/Device.hpp"
#include "ALCheck.hpp"

#include "glm/common.hpp" // max
#include "glm/gtc/type_ptr.hpp"

namespace audio {

Device::Device(const Config &config) {
  m_device = alcOpenDevice(nullptr);
  assert(m_device != nullptr);

  // clang-format off
  const ALCint attributes[] = {
    ALC_MONO_SOURCES, static_cast<ALCint>(glm::max(1u, config.maxNumSources)),
    0, 0,
  };
  // clang-format on
  m_context = alcCreateContext(m_device, attributes);
  assert(m_context != nullptr);
  const auto result = alcMakeContextCurrent(m_context);
  assert(result == ALC_TRUE);
}
Device::~Device() {
  alcMakeContextCurrent(nullptr);
  alcDestroyContext(m_context);
  m_context = nullptr;

  alcCloseDevice(m_device);
  m_device = nullptr;
}

Device &Device::setSettings(const Settings &settings) {
  return setDistanceModel(settings.distanceModel)
    .setDopplerFactor(settings.dopplerFactor)
    .setSpeedOfSound(settings.speedOfSound);
}
Device &Device::setDistanceModel(const DistanceModel distanceModel) {
  ALenum v{AL_NONE};
  if (distanceModel != DistanceModel::None) {
    v = std::to_underlying(distanceModel) + AL_DISTANCE_MODEL;
  }
  AL_CHECK(alDistanceModel(v));
  return *this;
}
Device &Device::setDopplerFactor(const float s) {
  AL_CHECK(alDopplerFactor(glm::max(0.0f, s)));
  return *this;
}
Device &Device::setSpeedOfSound(const float s) {
  AL_CHECK(alSpeedOfSound(glm::max(0.001f, s)));
  return *this;
}

Device &Device::setMasterGain(const float s) {
  AL_CHECK(alListenerf(AL_GAIN, glm::max(0.0f, s)));
  return *this;
}
Device &Device::setListenerTransform(const glm::vec3 &position,
                                     const glm::vec3 &forward,
                                     const glm::vec3 &up) {
  AL_CHECK(alListenerfv(AL_POSITION, glm::value_ptr(position)));
  const ALfloat orientation[] = {
    forward.x, forward.y, forward.z, up.x, up.y, up.z,
  };
  AL_CHECK(alListenerfv(AL_ORIENTATION, orientation));
  return *this;
}
Device &Device::setListenerVelocity(const glm::vec3 &v) {
  AL_CHECK(alListenerfv(AL_VELOCITY, glm::value_ptr(v)));
  return *this;
}

Device::Settings Device::getSettings() const {
  Settings settings{
    .distanceModel = DistanceModel::None,
    .dopplerFactor = alGetFloat(AL_DOPPLER_FACTOR),
    .speedOfSound = alGetFloat(AL_SPEED_OF_SOUND),
  };
  if (const auto v = alGetInteger(AL_DISTANCE_MODEL); v != AL_NONE) {
    settings.distanceModel = static_cast<DistanceModel>(v - AL_DISTANCE_MODEL);
  }
  assert(alGetError() == AL_NO_ERROR);
  return settings;
}

Buffer Device::createBuffer(const ClipInfo &info, const void *data) const {
  return Buffer{info, data};
}

//
// Helper:
//

const char *toString(const DistanceModel distanceModel) {
#define CASE(Value)                                                            \
  case DistanceModel::Value:                                                   \
    return #Value
  switch (distanceModel) {
    CASE(None);
    CASE(Inverse);
    CASE(InverseClamped);
    CASE(Linear);
    CASE(LinearClamped);
    CASE(Exponent);
    CASE(ExponentClamped);
  }

  assert(false);
  return "Undefined";
}

} // namespace audio
