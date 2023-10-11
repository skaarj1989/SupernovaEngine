#include "AudioPlayerComponent.hpp"

AudioPlayerComponent::AudioPlayerComponent(const SoundSettings &settings)
    : m_settings{settings} {}

AudioPlayerComponent &
AudioPlayerComponent::setStream(std::unique_ptr<audio::Decoder> &&decoder) {
  using namespace std::chrono_literals;
  m_streamPlayer->setStream(std::move(decoder), 250ms);
  return *this;
}

AudioPlayerComponent &
AudioPlayerComponent::setSettings(const SoundSettings &settings) {
  setPitch(settings.pitch)
    .setGain(settings.gain)
    .setMaxDistance(settings.maxDistance)
    .setRollOffFactor(settings.rollOffFactor)
    .setReferenceDistance(settings.referenceDistance)
    .setMinGain(settings.minGain)
    .setMaxGain(settings.maxGain)
    .setDirectional(settings.directional)
    .setLooping(settings.loop);
  return *this;
}
AudioPlayerComponent &AudioPlayerComponent::setPitch(const float s) {
  m_settings.pitch = s;
  m_streamPlayer->setPitch(s);
  return *this;
}
AudioPlayerComponent &AudioPlayerComponent::setGain(const float s) {
  m_settings.gain = s;
  m_streamPlayer->setGain(s);
  return *this;
}
AudioPlayerComponent &AudioPlayerComponent::setMaxDistance(const float s) {
  m_settings.maxDistance = s;
  m_streamPlayer->setMaxDistance(s);
  return *this;
}
AudioPlayerComponent &AudioPlayerComponent::setRollOffFactor(const float s) {
  m_settings.rollOffFactor = s;
  m_streamPlayer->setRollOffFactor(s);
  return *this;
}
AudioPlayerComponent &
AudioPlayerComponent::setReferenceDistance(const float s) {
  m_settings.referenceDistance = s;
  m_streamPlayer->setReferenceDistance(s);
  return *this;
}
AudioPlayerComponent &AudioPlayerComponent::setMinGain(const float s) {
  m_settings.minGain = s;
  m_streamPlayer->setMinGain(s);
  return *this;
}
AudioPlayerComponent &AudioPlayerComponent::setMaxGain(const float s) {
  m_settings.maxGain = s;
  m_streamPlayer->setMaxGain(s);
  return *this;
}
AudioPlayerComponent &AudioPlayerComponent::setDirectional(const bool b) {
  m_settings.directional = b;
  if (!m_settings.directional) {
    m_streamPlayer->setDirection(glm::vec3{0.0f});
  }
  return *this;
}
AudioPlayerComponent &AudioPlayerComponent::setLooping(const bool b) {
  m_settings.loop = b;
  m_streamPlayer->setLooping(b);
  return *this;
}

AudioPlayerComponent &AudioPlayerComponent::setVelocity(const glm::vec3 &v) {
  m_streamPlayer->setVelocity(v);
  return *this;
}

const SoundSettings &AudioPlayerComponent::getSettings() const {
  return m_settings;
}
glm::vec3 AudioPlayerComponent::getVelocity() const {
  return m_streamPlayer->getVelocity();
}

AudioPlayerComponent &AudioPlayerComponent::play() {
  m_streamPlayer->play();
  return *this;
}
AudioPlayerComponent &AudioPlayerComponent::pause() {
  m_streamPlayer->pause();
  return *this;
}
AudioPlayerComponent &AudioPlayerComponent::stop() {
  m_streamPlayer->stop();
  return *this;
}
