#pragma once

#include "SystemCommons.hpp"
#include "AudioWorld.hpp"

struct MainListener {
  entt::entity e{entt::null};
};

/*
  Context variables:
  - [creates] AudioWorld
  - [creates] MainListener
  Components:
  - Transform
  - ListenerComponent
  - [setup callbacks] SoundSourceComponent
  - [setup callbacks] AudioPlayerComponent
*/
class AudioSystem {
public:
  INTRODUCE_COMPONENTS(ListenerComponent, SoundSourceComponent,
                       AudioPlayerComponent)

  static void setup(entt::registry &, audio::Device &);
  static void update(entt::registry &, float dt);

  template <class Archive> static void save(Archive &archive) {
    auto &[registry, _] = cereal::get_user_data<OutputContext>(archive);
    auto &ctx = registry.ctx();
    archive(ctx.template get<AudioWorld>());
    archive(ctx.template get<MainListener>().e);
  }
  template <class Archive> static void load(Archive &archive) {
    auto &[registry, _] = cereal::get_user_data<InputContext>(archive);
    auto &ctx = registry.ctx();
    archive(ctx.template get<AudioWorld>());
    archive(ctx.template get<MainListener>().e);
  }
};

[[nodiscard]] AudioWorld &getAudioWorld(entt::registry &);
[[nodiscard]] MainListener &getMainListener(entt::registry &);
