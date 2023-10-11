#pragma once

#include "audio/Source.hpp"
#include "SoundSettings.hpp"
#include "AudioClipManager.hpp"

class SoundSourceComponent {
  friend class AudioWorld;

  static constexpr auto in_place_delete = true;

public:
  explicit SoundSourceComponent(const SoundSettings & = {});

  SoundSourceComponent &setClip(std::shared_ptr<AudioClipResource>);

  SoundSourceComponent &setSettings(const SoundSettings &);
  SoundSourceComponent &setPitch(const float);
  SoundSourceComponent &setGain(const float);
  SoundSourceComponent &setMaxDistance(const float);
  SoundSourceComponent &setRollOffFactor(const float);
  SoundSourceComponent &setReferenceDistance(const float);
  SoundSourceComponent &setMinGain(const float);
  SoundSourceComponent &setMaxGain(const float);
  SoundSourceComponent &setDirectional(const bool);
  SoundSourceComponent &setLooping(const bool);

  SoundSourceComponent &setVelocity(const glm::vec3 &);

  [[nodiscard]] std::shared_ptr<AudioClipResource> getClip() const;
  [[nodiscard]] const SoundSettings &getSettings() const;
  [[nodiscard]] glm::vec3 getVelocity() const;

  SoundSourceComponent &play();
  SoundSourceComponent &pause();
  SoundSourceComponent &stop();

  template <typename Archive> void save(Archive &archive) const {
    archive(m_settings, serialize(m_resource));
  }
  template <typename Archive> void load(Archive &archive) {
    archive(m_settings);

    std::optional<std::string> path;
    archive(path);
    if (path) m_resource = loadResource<AudioClipManager>(*path);
  }

private:
  SoundSettings m_settings;
  std::shared_ptr<AudioClipResource> m_resource;
  std::shared_ptr<audio::Source> m_source;
};

static_assert(std::is_copy_constructible_v<SoundSourceComponent>);
