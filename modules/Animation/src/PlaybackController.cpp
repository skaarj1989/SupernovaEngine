#include "animation/PlaybackController.hpp"
#include "glm/common.hpp" // clamp

void PlaybackController::update(const ozz::animation::Animation &animation,
                                float dt) {
  auto newTime = m_timeRatio;
  if (m_playing)
    newTime = m_timeRatio + dt * m_playbackSpeed / animation.duration();

  setTimeRatio(newTime);
}

PlaybackController &PlaybackController::setTimeRatio(float ratio) {
  m_previousTimeRatio = m_timeRatio;
  m_timeRatio =
    m_loop ? ratio - std::floorf(ratio) : glm::clamp(ratio, 0.0f, 1.0f);
  return *this;
}
PlaybackController &PlaybackController::setSpeed(float f) {
  m_playbackSpeed = f;
  return *this;
}
PlaybackController &PlaybackController::setLoop(bool b) {
  m_loop = b;
  return *this;
}

float PlaybackController::getTimeRatio() const { return m_timeRatio; }
float PlaybackController::getPreviousTimeRatio() const {
  return m_previousTimeRatio;
}

float PlaybackController::getPlaybackSpeed() const { return m_playbackSpeed; }

bool PlaybackController::isPlaying() const { return m_playing; }
bool PlaybackController::isLooped() const { return m_loop; }

PlaybackController &PlaybackController::play() {
  if (!m_playing) {
    m_previousTimeRatio = m_timeRatio = 0.0f;
    m_playing = true;
  }
  return *this;
}
PlaybackController &PlaybackController::pause() {
  m_playing = false;
  return *this;
}
PlaybackController &PlaybackController::resume() {
  m_playing = true;
  return *this;
}
PlaybackController &PlaybackController::stop() {
  m_previousTimeRatio = m_timeRatio = 0.0f;
  m_playing = false;
  return *this;
}

PlaybackController &PlaybackController::reset() {
  stop();
  m_playbackSpeed = 1.0f;
  return *this;
}
