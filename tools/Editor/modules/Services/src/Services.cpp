#include "Services.hpp"

#include "renderer/MeshManager.hpp"
#include "animation/SkeletonManager.hpp"
#include "animation/AnimationManager.hpp"
#include "physics/ColliderManager.hpp"
#include "AudioClipManager.hpp"
#include "ScriptManager.hpp"

void Services::init(rhi::RenderDevice &rd, audio::Device &ad) {
  Resources::Skeletons::emplace();
  Resources::Animations::emplace();

  Resources::Colliders::emplace();

  auto &textureManager = Resources::Textures::emplace(rd);
  auto &materialManager = Resources::Materials::emplace(textureManager);
  Resources::Meshes::emplace(rd, materialManager);

  Resources::AudioClips::emplace(ad);

  Resources::Scripts::emplace();
}
void Services::reset() {
  Resources::Scripts::reset();

  Resources::AudioClips::reset();

  Resources::Colliders::reset();

  Resources::Animations::reset();
  Resources::Skeletons::reset();

  Resources::Meshes::reset();
  Resources::Materials::reset();
  Resources::Textures::reset();
}

void Services::Resources::clear() {
  Textures::value().clear();
  Materials::value().clear();
  Meshes::value().clear();

  AudioClips::value().clear();

  Skeletons::value().clear();
  Animations::value().clear();

  Colliders::value().clear();

  Scripts::value().clear();
}
