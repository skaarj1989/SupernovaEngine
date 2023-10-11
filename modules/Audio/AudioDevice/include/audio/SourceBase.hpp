#pragma once

#include "AL/al.h"
#include "glm/ext/vector_float3.hpp"

namespace audio {

class SourceBase {
public:
  enum class State {
    Initial,
    Playing,
    Paused,
    Stopped,
  };

  SourceBase(const SourceBase &) = delete;
  SourceBase(SourceBase &&) noexcept;
  virtual ~SourceBase();

  SourceBase &operator=(const SourceBase &) = delete;
  SourceBase &operator=(SourceBase &&) noexcept;

  void play();
  void pause();
  void stop();

  void setPitch(const float);
  void setGain(const float);
  void setMaxDistance(const float);
  void setRollOffFactor(const float);
  void setReferenceDistance(const float);
  void setMinGain(const float);
  void setMaxGain(const float);

  void setPosition(const glm::vec3 &);
  void setDirection(const glm::vec3 &);
  void setVelocity(const glm::vec3 &);

  [[nodiscard]] float getPitch() const;
  [[nodiscard]] float getGain() const;
  [[nodiscard]] float getMaxDistance() const;
  [[nodiscard]] float getRollOffFactor() const;
  [[nodiscard]] float getReferenceDistance() const;
  [[nodiscard]] float getMinGain() const;
  [[nodiscard]] float getMaxGain() const;

  [[nodiscard]] glm::vec3 getPosition() const;
  [[nodiscard]] glm::vec3 getDirection() const;
  [[nodiscard]] glm::vec3 getVelocity() const;

  [[nodiscard]] State getState() const;

  [[nodiscard]] bool isPlaying() const;
  [[nodiscard]] bool isPaused() const;
  [[nodiscard]] bool isStopped() const;

protected:
  SourceBase();

  void _destroy();

protected:
  ALuint m_id{AL_NONE};
};

} // namespace audio
