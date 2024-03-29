#include "physics/JoltPhysics.hpp"
#include "physics/DebugRenderer.hpp"

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "Jolt/Jolt.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/Factory.h"
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

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
