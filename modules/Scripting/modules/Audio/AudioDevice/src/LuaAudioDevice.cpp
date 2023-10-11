#include "LuaAudioDevice.hpp"
#include "sol/state.hpp"

#include "audio/Device.hpp"
#include "audio/Utility.hpp"

#include "Sol2HelperMacros.hpp"

using namespace audio;

namespace {

void registerFreeFunctions(sol::table &lua) {
  lua["dBToVolume"] = dBToVolume;
  lua["volumeTodB"] = volumeTodB;
}
void registerBuffer(sol::table &lua) {
  // clang-format off
#define MAKE_PAIR(Key) _MAKE_PAIR(NumChannels, Key)
  DEFINE_ENUM(NumChannels, {
    MAKE_PAIR(Invalid),
    MAKE_PAIR(Mono),
    MAKE_PAIR(Stereo),
    MAKE_PAIR(Surround1D),
    MAKE_PAIR(QuadSurround),
    MAKE_PAIR(Surround),
    MAKE_PAIR(Surround5_1),
    MAKE_PAIR(Surround6_1),
    MAKE_PAIR(Surround7_1),
  });
#undef MAKE_PAIR

#define BIND(Member) _BIND(ClipInfo, Member)
  DEFINE_USERTYPE(ClipInfo,
    BIND(numChannels),
    BIND(bitsPerSample),
    BIND(sampleRate),
    BIND(numSamples),

    BIND(blockAlign),
    BIND(byteRate),
    BIND(dataSize),
    BIND(duration),

    BIND_TOSTRING(ClipInfo)
  );
#undef BIND

  DEFINE_USERTYPE(Buffer,
    sol::no_constructor,
    _BIND(Buffer, getInfo)
  );
  // clang-format on
}
void registerDecoder(sol::table &lua) {
#define BIND(Member) _BIND(Decoder, Member)
  // clang-format off
  DEFINE_USERTYPE(Decoder,
    sol::no_constructor,

    BIND(isOpen),
    BIND(getInfo),

    sol::meta_function::to_string, [](const Decoder &self) {
      return std::format("AudioDecoder()", self.isOpen() ? "open" : "closed");
    }
  );
#undef BIND
  // clang-format on
}

} // namespace

void registerAudioDevice(sol::state &lua) {
  auto m = lua["audio"].get_or_create<sol::table>();

  // clang-format off
#define MAKE_PAIR(Key) _MAKE_PAIR(DistanceModel, Key)
  m.new_enum<DistanceModel>("DistanceModel", {
    MAKE_PAIR(None),
    MAKE_PAIR(Inverse),
    MAKE_PAIR(InverseClamped),
    MAKE_PAIR(Linear),
    MAKE_PAIR(LinearClamped),
    MAKE_PAIR(Exponent),
    MAKE_PAIR(ExponentClamped),
  });
#undef MAKE_PAIR

#define BIND(Member) _BIND(Device, Member)
  m.new_usertype<Device>("Device",
    sol::no_constructor,

    BIND(setSettings),
    BIND(setDistanceModel),
    BIND(setDopplerFactor),
    BIND(setSpeedOfSound),
    BIND(setMasterGain),

    BIND(getSettings),
    
    BIND_TOSTRING(Device)
  );
#undef BIND

#define CAPTURE_FIELD(name, defaultValue)                                      \
  .##name = t.get_or(#name, defaultValue)

#define BIND(Member) _BIND(Device::Settings, Member)
  m["Device"].get<sol::table>().new_usertype<Device::Settings>("Settings",
    sol::call_constructor,
    sol::factories(
      [] { return Device::Settings{}; },
      [](const sol::table &t) {
        return Device::Settings{
          CAPTURE_FIELD(distanceModel, DistanceModel::InverseClamped),
          CAPTURE_FIELD(dopplerFactor, 1.0f),
          CAPTURE_FIELD(speedOfSound, 343.3f)
        };
      }
    ),

    BIND(distanceModel),
    BIND(dopplerFactor),
    BIND(speedOfSound),

    sol::meta_function::to_string, [] { return "AudioDevice.Settings"; }
  );
#undef BIND
  // clang-format on

  registerFreeFunctions(m);
  registerBuffer(m);
  registerDecoder(m);
}
