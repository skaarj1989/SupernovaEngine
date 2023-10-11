#pragma once

#include "audio/Device.hpp"
#include "SoundSourceComponent.hpp"
#include "AudioPlayerComponent.hpp"
#include "Transform.hpp"

struct ListenerComponent {
  static constexpr auto in_place_delete = true;

  glm::vec3 velocity{0.0f};

  template <class Archive> void serialize(Archive &) {}
};

class AudioWorld {
public:
  explicit AudioWorld(audio::Device &);

  void setSettings(const audio::Device::Settings &);
  [[nodiscard]] const audio::Device::Settings &getSettings() const;

  void init(const Transform *, SoundSourceComponent &) const;
  void init(const Transform *, AudioPlayerComponent &) const;

  void updateListener(const Transform &, const ListenerComponent &);
  void update(const Transform &, SoundSourceComponent &) const;
  void update(const Transform *, AudioPlayerComponent &) const;

  template <class Archive> void save(Archive &archive) const {
    archive(m_settings);
  }
  template <class Archive> void load(Archive &archive) {
    audio::Device::Settings settings;
    archive(settings);
    setSettings(settings);
  }

  audio::Device &getDevice() const { return m_device; };

private:
  audio::Device &m_device;
  audio::Device::Settings m_settings;
};

[[nodiscard]] std::unique_ptr<audio::Decoder>
createStream(const std::filesystem::path &);
