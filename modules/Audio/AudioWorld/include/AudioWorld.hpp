#pragma once

#include "audio/Device.hpp"
#include "Decoder.hpp"
#include "entt/core/type_info.hpp"
#include "glm/ext/vector_float3.hpp"
#include <memory>
#include <filesystem>

struct ListenerComponent {
  static constexpr auto in_place_delete = true;

  glm::vec3 velocity{0.0f};

  template <class Archive> void serialize(Archive &) {}
};

static_assert(std::is_copy_constructible_v<ListenerComponent>);

template <> struct entt::type_hash<ListenerComponent> {
  [[nodiscard]] static constexpr entt::id_type value() noexcept {
    return 3293377831;
  }
};

class Transform;
class SoundSourceComponent;
class AudioPlayerComponent;

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
