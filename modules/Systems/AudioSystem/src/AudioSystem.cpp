#include "AudioSystem.hpp"
#include "Transform.hpp"
#include "tracy/Tracy.hpp"

namespace {

template <class T> void initComponent(entt::registry &r, const entt::entity e) {
  const auto &world = r.ctx().get<AudioWorld>();
  world.init(r.try_get<Transform>(e), r.get<T>(e));
}
template <class T> void connectComponent(entt::registry &r) {
  r.on_construct<T>().template connect<&initComponent<T>>();
}

} // namespace

//
// AudioSystem class:
//

void AudioSystem::setup(entt::registry &r, audio::Device &device) {
  r.ctx().emplace<AudioWorld>(device);
  r.ctx().emplace<MainListener>();

  connectComponent<SoundSourceComponent>(r);
  connectComponent<AudioPlayerComponent>(r);
}
void AudioSystem::update(entt::registry &r, [[maybe_unused]] const float) {
  ZoneScopedN("AudioSystem::Update");
  auto &world = r.ctx().get<AudioWorld>();

  if (const auto [e] = getMainListener(r); e != entt::null) {
    auto *xf = r.try_get<const Transform>(e);
    auto *listener = r.try_get<const ListenerComponent>(e);
    if (xf && listener) {
      world.updateListener(*xf, *listener);
    }
  }

  for (const auto [e, c] : r.view<SoundSourceComponent>().each()) {
    if (auto *xf = r.try_get<const Transform>(e)) world.update(*xf, c);
  }
  for (const auto [e, c] : r.view<AudioPlayerComponent>().each()) {
    world.update(r.try_get<const Transform>(e), c);
  }
}

//
// Helper:
//

AudioWorld &getAudioWorld(entt::registry &r) {
  return r.ctx().get<AudioWorld>();
}
MainListener &getMainListener(entt::registry &r) {
  return r.ctx().get<MainListener>();
}
