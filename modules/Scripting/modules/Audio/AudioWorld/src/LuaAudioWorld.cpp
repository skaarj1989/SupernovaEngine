#include "LuaAudioWorld.hpp"
#include "sol/state.hpp"

#include "os/FileSystem.hpp"
#include "AudioWorld.hpp"
#include "SoundSourceComponent.hpp"
#include "AudioPlayerComponent.hpp"

#include "Sol2HelperMacros.hpp"

namespace {

void registerResource(sol::state &lua) {
  // clang-format off
  lua.DEFINE_USERTYPE(AudioClipResource,
    sol::no_constructor,
    sol::base_classes, sol::bases<Resource, audio::Buffer>(),

    BIND_TOSTRING(AudioClipResource)
  );
  // clang-format on
  lua["loadAudioClip"] = loadResource<AudioClipManager>;
}

void registerListenerComponent(sol::state &lua) {
  // clang-format off
#define BIND(Member) _BIND(ListenerComponent, Member)
  lua.DEFINE_USERTYPE(ListenerComponent,
    sol::call_constructor,
    sol::factories([] { return ListenerComponent{}; }),

    BIND(velocity),

    BIND_TYPEID(ListenerComponent),
    BIND_TOSTRING(ListenerComponent)
  );
#undef BIND
  // clang-format on
}

void registerSoundSettings(sol::state &lua) {
#define BIND(Member) _BIND(SoundSettings, Member)
  // clang-format off
  lua.DEFINE_USERTYPE(SoundSettings,
    sol::call_constructor,
    sol::factories(
      [] { return SoundSettings{}; },
      [](const sol::table &t) {
        return SoundSettings{
          CAPTURE_FIELD(pitch, 1.0f),
          CAPTURE_FIELD(gain, 1.0f),
          CAPTURE_FIELD(maxDistance, std::numeric_limits<float>::max()),
          CAPTURE_FIELD(rollOffFactor, 1.0f),
          CAPTURE_FIELD(referenceDistance, 1.0f),
          CAPTURE_FIELD(minGain, 0.0f),
          CAPTURE_FIELD(maxGain, 1.0f),
          CAPTURE_FIELD(directional, false),
          CAPTURE_FIELD(loop, false),
        };
      }
    ),

    BIND(pitch),
    BIND(gain),
    BIND(maxDistance),
    BIND(rollOffFactor),
    BIND(referenceDistance),
    BIND(minGain),
    BIND(maxGain),
    BIND(directional),
    BIND(loop),
    BIND(playing)
  );
  // clang-format on
#undef BIND
}
void registerSoundSourceComponent(sol::state &lua) {
  // clang-format off
#define BIND(Member) _BIND(SoundSourceComponent, Member)
  lua.DEFINE_USERTYPE(SoundSourceComponent,
    sol::call_constructor,
    sol::constructors<
      SoundSourceComponent(),
      SoundSourceComponent(const SoundSettings &)
    >(),

    BIND(setClip),

    BIND(setSettings),
    BIND(setPitch),
    BIND(setGain),
    BIND(setMaxDistance),
    BIND(setRollOffFactor),
    BIND(setReferenceDistance),
    BIND(setMinGain),
    BIND(setMaxGain),
    BIND(setDirectional),
    BIND(setLooping),

    BIND(setVelocity),

    BIND(getClip),
    BIND(getSettings),
    BIND(getVelocity),

    BIND(play),
    BIND(pause),
    BIND(stop),
  
    BIND_TYPEID(SoundSourceComponent),
    BIND_TOSTRING(SoundSourceComponent)
  );
#undef BIND
  // clang-format on
}
void registerAudioPlayerComponent(sol::state &lua) {
#define BIND(Member) _BIND(AudioPlayerComponent, Member)
  // clang-format off
  lua.DEFINE_USERTYPE(AudioPlayerComponent,
    sol::call_constructor,
    sol::constructors<
      AudioPlayerComponent(),
      AudioPlayerComponent(const SoundSettings &)
    >(),

    "setStream",
      [](AudioPlayerComponent &self, std::unique_ptr<audio::Decoder> &decoder) {
        self.setStream(std::move(decoder));
      },

    BIND(setSettings),
    BIND(setPitch),
    BIND(setGain),
    BIND(setMaxDistance),
    BIND(setRollOffFactor),
    BIND(setReferenceDistance),
    BIND(setMinGain),
    BIND(setMaxGain),
    BIND(setDirectional),
    BIND(setLooping),

    BIND(setVelocity),

    BIND(getSettings),
    BIND(getVelocity),

    BIND(play),
    BIND(pause),
    BIND(stop),

    BIND_TYPEID(AudioPlayerComponent),
    BIND_TOSTRING(AudioPlayerComponent)
  );
  // clang-format on
#undef BIND

  lua["createStream"] = [](const std::string_view path) {
    return createStream(os::FileSystem::getRoot() / path);
  };
}

} // namespace

void registerAudioWorld(sol::state &lua) {
  // clang-format off
#define BIND(Member) _BIND(AudioWorld, Member)
  lua.DEFINE_USERTYPE(AudioWorld,
    sol::no_constructor,

    BIND(setSettings),
    BIND(getSettings),
    
    BIND(getDevice),

    BIND_TOSTRING(AudioWorld)
  );
#undef BIND
  // clang-format on

  registerResource(lua);
  registerListenerComponent(lua);

  registerSoundSettings(lua);
  registerSoundSourceComponent(lua);
  registerAudioPlayerComponent(lua);
}
