#include "physics/JoltPhysics.hpp"
#include "physics/DebugRenderer.hpp"

#include "Jolt/Jolt.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/Factory.h"

#include <cassert>

DebugRenderer *JoltPhysics::debugRenderer = nullptr;

void JoltPhysics::setup() {
  JPH::RegisterDefaultAllocator();

  assert(JPH::Factory::sInstance == nullptr);
  JPH::Factory::sInstance = new JPH::Factory;

  JPH::RegisterTypes();

  JoltPhysics::debugRenderer = new DebugRenderer;
}
void JoltPhysics::cleanup() {
  delete JoltPhysics::debugRenderer;

  assert(JPH::Factory::sInstance != nullptr);
  delete JPH::Factory::sInstance;
  JPH::Factory::sInstance = nullptr;
}
