#include "SoundSourceComponent.hpp"

SoundSourceComponent::SoundSourceComponent(const SoundSettings &settings)
    : m_settings{settings} {}

SoundSourceComponent &
SoundSourceComponent::setClip(std::shared_ptr<AudioClipResource> clip) {
  m_resource = std::move(clip);
  if (m_source) m_source->setBuffer(m_resource);
  return *this;
}

SoundSourceComponent &
SoundSourceComponent::setSettings(const SoundSettings &settings) {
  setPitch(settings.pitch)
    .setGain(settings.gain)
    .setMaxDistance(settings.maxDistance)
    .setRollOffFactor(settings.rollOffFactor)
    .setReferenceDistance(settings.referenceDistance)
    .setMinGain(settings.minGain)
    .setMaxGain(settings.maxGain)
    .setDirectional(settings.directional)
    .setLooping(settings.loop);
  if (settings.playing) play();
  return *this;
}
SoundSourceComponent &SoundSourceComponent::setPitch(const float s) {
  m_settings.pitch = s;
  m_source->setPitch(s);
  return *this;
}
SoundSourceComponent &SoundSourceComponent::setGain(const float s) {
  m_settings.gain = s;
  m_source->setGain(s);
  return *this;
}
SoundSourceComponent &SoundSourceComponent::setMaxDistance(const float s) {
  m_settings.maxDistance = s;
  m_source->setMaxDistance(s);
  return *this;
}
SoundSourceComponent &SoundSourceComponent::setRollOffFactor(const float s) {
  m_settings.rollOffFactor = s;
  m_source->setRollOffFactor(s);
  return *this;
}
SoundSourceComponent &
SoundSourceComponent::setReferenceDistance(const float s) {
  m_settings.referenceDistance = s;
  m_source->setReferenceDistance(s);
  return *this;
}
SoundSourceComponent &SoundSourceComponent::setMinGain(const float s) {
  m_settings.minGain = s;
  m_source->setMinGain(s);
  return *this;
}
SoundSourceComponent &SoundSourceComponent::setMaxGain(const float s) {
  m_settings.maxGain = s;
  m_source->setMaxGain(s);
  return *this;
}
SoundSourceComponent &SoundSourceComponent::setDirectional(const bool b) {
  m_settings.directional = b;
  if (!m_settings.directional) {
    m_source->setDirection(glm::vec3{0.0f});
  }
  return *this;
}
SoundSourceComponent &SoundSourceComponent::setLooping(const bool b) {
  m_settings.loop = b;
  m_source->setLooping(b);
  return *this;
}

SoundSourceComponent &SoundSourceComponent::setVelocity(const glm::vec3 &v) {
  m_source->setVelocity(v);
  return *this;
}

std::shared_ptr<AudioClipResource> SoundSourceComponent::getClip() const {
  return m_resource;
}
const SoundSettings &SoundSourceComponent::getSettings() const {
  return m_settings;
}
glm::vec3 SoundSourceComponent::getVelocity() const {
  return m_source->getVelocity();
}

SoundSourceComponent &SoundSourceComponent::play() {
  m_source->play();
  return *this;
}
SoundSourceComponent &SoundSourceComponent::pause() {
  m_source->pause();
  return *this;
}
SoundSourceComponent &SoundSourceComponent::stop() {
  m_source->stop();
  return *this;
}
