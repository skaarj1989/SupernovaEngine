#pragma once

#include "entt/core/type_info.hpp"

namespace ozz::animation {
class Animation;
}

class PlaybackController {
public:
  PlaybackController() = default;

  void update(const ozz::animation::Animation &, float dt);

  PlaybackController &setTimeRatio(float);
  PlaybackController &setSpeed(float);
  PlaybackController &setLoop(bool);

  float getTimeRatio() const;
  float getPreviousTimeRatio() const;
  float getPlaybackSpeed() const;

  bool isPlaying() const;
  bool isLooped() const;

  PlaybackController &play();
  PlaybackController &pause();
  PlaybackController &resume();
  PlaybackController &stop();

  PlaybackController &reset();

  template <class Archive> void serialize(Archive &archive) {
    archive(m_timeRatio, m_previousTimeRatio, m_playbackSpeed, m_playing,
            m_loop);
  }

private:
  float m_timeRatio{0.0f};
  float m_previousTimeRatio{0.0f};
  float m_playbackSpeed{1.0f};

  bool m_playing{true};
  bool m_loop{true};
};

template <> struct entt::type_hash<PlaybackController> {
  [[nodiscard]] static constexpr entt::id_type value() noexcept {
    return 820180132;
  }
};
