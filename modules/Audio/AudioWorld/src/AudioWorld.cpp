#include "AudioWorld.hpp"
#include "os/FileSystem.hpp"
#include "AudioClipLoader.hpp"
#include "spdlog/spdlog.h"

namespace {

void synchronize(audio::SourceBase &source, const Transform &xf,
                 const bool directional) {
  source.setPosition(xf.getPosition());
  if (directional) source.setDirection(xf.getForward());
}

} // namespace

//
// AudioWorld class:
//

AudioWorld::AudioWorld(audio::Device &device) : m_device{device} {}

void AudioWorld::setSettings(const audio::Device::Settings &settings) {
  m_device.setSettings(settings);
  m_settings = m_device.getSettings();
}
const audio::Device::Settings &AudioWorld::getSettings() const {
  return m_settings;
}

void AudioWorld::init(const Transform *xf, SoundSourceComponent &c) const {
  c.m_source = std::make_shared<audio::Source>(c.m_resource);
  c.setSettings(c.m_settings);
  if (xf) update(*xf, c);
}
void AudioWorld::init(const Transform *xf, AudioPlayerComponent &c) const {
  c.m_streamPlayer = std::make_shared<audio::StreamPlayer>(3);
  if (xf) update(xf, c);
}

void AudioWorld::updateListener(const Transform &xf,
                                const ListenerComponent &listener) {
  m_device.setListenerTransform(xf.getPosition(), xf.getForward(), xf.getUp())
    .setListenerVelocity(listener.velocity);
}
void AudioWorld::update(const Transform &xf, SoundSourceComponent &c) const {
  synchronize(*c.m_source, xf, c.getSettings().directional);
  c.m_settings.playing = c.m_source->isPlaying();
}
void AudioWorld::update(const Transform *xf, AudioPlayerComponent &c) const {
  if (xf) synchronize(*c.m_streamPlayer, *xf, c.getSettings().directional);
  c.m_streamPlayer->update();
  c.m_settings.playing = c.m_streamPlayer->isPlaying();
}

//
// Helper:
//

std::unique_ptr<audio::Decoder> createStream(const std::filesystem::path &p) {
  if (auto decoder = createDecoder(p)) {
    return std::move(*decoder);
  } else {
    SPDLOG_ERROR("{}: {}", os::FileSystem::relativeToRoot(p)->generic_string(),
                 decoder.error());
    return nullptr;
  }
}
