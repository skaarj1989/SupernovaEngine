#include "AudioClipManager.hpp"
#include "audio/Device.hpp"
#include "LoaderHelper.hpp"

AudioClipManager::AudioClipManager(audio::Device &device) : m_device{device} {}

AudioClipResourceHandle AudioClipManager::load(const std::filesystem::path &p) {
  return ::load(*this, p, LoadMode::External, m_device);
}
