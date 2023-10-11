#pragma once

#include "AudioClipLoader.hpp"
#include "entt/resource/cache.hpp"

using AudioClipCache = entt::resource_cache<AudioClipResource, AudioClipLoader>;

class AudioClipManager final : public AudioClipCache {
public:
  explicit AudioClipManager(audio::Device &);
  ~AudioClipManager() = default;

  [[nodiscard]] AudioClipResourceHandle load(const std::filesystem::path &);

private:
  audio::Device &m_device;
};
