#include "physics/RigidBody.hpp"
#include "physics/Conversion.hpp"

RigidBody::RigidBody(const Settings &settings) : m_settings{settings} {}

RigidBody::operator bool() const {
  return m_joltPhysics && !m_bodyId.IsInvalid();
}

const RigidBody::Settings &RigidBody::getSettings() const { return m_settings; }

#define BODY_INTERFACE m_joltPhysics->GetBodyInterface()
#define MUTATOR(Func, Value) BODY_INTERFACE.Func(m_bodyId, to_Jolt(Value))
#define GETTER(Func) to_glm(BODY_INTERFACE.Get##Func(m_bodyId))

JPH::BodyID RigidBody::getBodyId() const { return m_bodyId; }

void RigidBody::setLinearVelocity(const glm::vec3 &v) {
  MUTATOR(SetLinearVelocity, v);
}
glm::vec3 RigidBody::getLinearVelocity() const {
  return GETTER(LinearVelocity);
}

void RigidBody::applyImpulse(const glm::vec3 &v) { MUTATOR(AddImpulse, v); }
