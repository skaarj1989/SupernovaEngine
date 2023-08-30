#pragma once

#include "CollisionLayer.hpp"

enum class MotionType { Dynamic, Static, Kinematic };

struct RigidBodySettings {
  CollisionLayer layer;
  MotionType motionType{MotionType::Static};
  float friction{0.2f};
  float restitution{0.0f};
  float linearDamping{0.05f};
  float angularDamping{0.05f};
  float gravityFactor{1.0f};

  template <class Archive> void serialize(Archive &archive) {
    archive(layer, motionType, friction, restitution, linearDamping,
            angularDamping, gravityFactor);
  }
};
