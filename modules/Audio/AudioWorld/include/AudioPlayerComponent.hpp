#pragma once

#include "entt/core/type_info.hpp"
#include "audio/StreamPlayer.hpp"
#include "SoundSettings.hpp"

class AudioPlayerComponent {
  friend class AudioWorld;

  static constexpr auto in_place_delete = true;

public:
  explicit AudioPlayerComponent(const SoundSettings & = {});

  AudioPlayerComponent &setStream(std::unique_ptr<audio::Decoder> &&);

  AudioPlayerComponent &setSettings(const SoundSettings &);
  AudioPlayerComponent &setPitch(const float);
  AudioPlayerComponent &setGain(const float);
  AudioPlayerComponent &setMaxDistance(const float);
  AudioPlayerComponent &setRollOffFactor(const float);
  AudioPlayerComponent &setReferenceDistance(const float);
  AudioPlayerComponent &setMinGain(const float);
  AudioPlayerComponent &setMaxGain(const float);
  AudioPlayerComponent &setDirectional(const bool);
  AudioPlayerComponent &setLooping(const bool);

  AudioPlayerComponent &setVelocity(const glm::vec3 &);

  [[nodiscard]] const SoundSettings &getSettings() const;
  [[nodiscard]] glm::vec3 getVelocity() const;

  AudioPlayerComponent &play();
  AudioPlayerComponent &pause();
  AudioPlayerComponent &stop();

  template <typename Archive> void serialize(Archive &archive) {
    archive(m_settings);
  }

private:
  SoundSettings m_settings;
  std::shared_ptr<audio::StreamPlayer> m_streamPlayer;
};

static_assert(std::is_copy_constructible_v<AudioPlayerComponent>);

template <> struct entt::type_hash<AudioPlayerComponent> {
  [[nodiscard]] static constexpr entt::id_type value() noexcept {
    return 2315303163;
  }
};
