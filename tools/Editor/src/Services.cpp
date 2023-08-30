#include "Services.hpp"

void Services::init(rhi::RenderDevice &rd) {
  Resources::Skeletons::emplace();
  Resources::Animations::emplace();

  Resources::Colliders::emplace();

  auto &textureManager = Resources::Textures::emplace(rd);
  auto &materialManager = Resources::Materials::emplace(textureManager);
  Resources::Meshes::emplace(rd, materialManager);

  Resources::Scripts::emplace();
}
void Services::reset() {
  Resources::Scripts::reset();

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

  Skeletons::value().clear();
  Animations::value().clear();

  Colliders::value().clear();

  Scripts::value().clear();
}
