#include "LuaAudioDevice.hpp"
#include "sol/state.hpp"

#include "Decoder.hpp"
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
  lua.DEFINE_ENUM(NumChannels, {
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
  lua.DEFINE_USERTYPE(ClipInfo,
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

  lua.DEFINE_USERTYPE(Buffer,
    sol::no_constructor,
    _BIND(Buffer, getInfo),
    BIND_TOSTRING(Buffer)
  );
  // clang-format on
}
void registerDecoder(sol::table &lua) {
#define BIND(Member) _BIND(Decoder, Member)
  // clang-format off
  lua.DEFINE_USERTYPE(Decoder,
    sol::no_constructor,

    BIND(isOpen),
    BIND(getInfo),

    sol::meta_function::to_string, [](const Decoder &self) {
      return std::format("AudioDecoder({})", self.isOpen() ? "open" : "closed");
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
  m.DEFINE_ENUM(DistanceModel, {
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
  m.DEFINE_USERTYPE(Device,
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

#define BIND(Member) _BIND(Device::Settings, Member)
  m DEFINE_NESTED_USERTYPE(Device, Settings,
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

    BIND_TOSTRING(Device::Settings)
  );
#undef BIND
  // clang-format on

  registerFreeFunctions(m);
  registerBuffer(m);
  registerDecoder(m);
}
