#pragma once

#include "CollisionLayer.hpp"

struct CharacterSettings {
  float maxSlopeAngle{50.0f}; // in degrees
  CollisionLayer layer;
  float mass{80.0f};
  float friction{0.2f};
  float gravityFactor{1.0f};

  template <class Archive> void serialize(Archive &archive) {
    archive(maxSlopeAngle, layer, mass, friction, gravityFactor);
  }
};
