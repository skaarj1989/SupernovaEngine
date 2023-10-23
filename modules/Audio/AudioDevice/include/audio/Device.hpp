#pragma once

#include "Source.hpp"
#include "StreamPlayer.hpp"
#include "AL/alc.h"

namespace audio {

enum class DistanceModel {
  None = 0,
  Inverse,
  InverseClamped,
  Linear,
  LinearClamped,
  Exponent,
  ExponentClamped,
};

class Device {
public:
  struct Config {
    uint32_t maxNumSources{255};
  };

#ifdef __GNUG__
  static constexpr Config defaultConfig() { return Config{}; }
  explicit Device(const Config & = defaultConfig());
#else
  explicit Device(const Config & = {});
#endif
  Device(const Device &) = delete;
  Device(Device &&) noexcept = default;
  ~Device();

  Device &operator=(const Device &) = delete;
  Device &operator=(Device &&) noexcept = default;

  struct Settings {
    DistanceModel distanceModel{DistanceModel::InverseClamped};
    float dopplerFactor{1.0f};
    float speedOfSound{343.3f};

    template <class Archive> void serialize(Archive &archive) {
      archive(distanceModel, dopplerFactor, speedOfSound);
    }
  };

  Device &setSettings(const Settings &);
  Device &setDistanceModel(const DistanceModel);
  Device &setDopplerFactor(const float);
  Device &setSpeedOfSound(const float);

  Device &setMasterGain(const float);
  Device &setListenerTransform(const glm::vec3 &position,
                               const glm::vec3 &forward, const glm::vec3 &up);
  Device &setListenerVelocity(const glm::vec3 &);

  [[nodiscard]] Settings getSettings() const;

  [[nodiscard]] Buffer createBuffer(const ClipInfo &, const void *data) const;

private:
  ALCdevice *m_device{nullptr};
  ALCcontext *m_context{nullptr};
};

[[nodiscard]] const char *toString(const DistanceModel);

} // namespace audio
