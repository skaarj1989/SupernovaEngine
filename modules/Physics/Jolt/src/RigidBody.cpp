#include "physics/RigidBody.hpp"
#include "physics/Conversion.hpp"

RigidBody::RigidBody(const RigidBodySettings &settings)
    : m_settings{settings} {}

void RigidBody::setCollisionShape(const JPH::Shape *shape) {
  assert(shape);
  m_joltPhysics->GetBodyInterface().SetShape(m_bodyId, shape, false,
                                             JPH::EActivation::DontActivate);
}

const RigidBodySettings &RigidBody::getSettings() const { return m_settings; }

void RigidBody::setLinearVelocity(const glm::vec3 &v) {
  m_joltPhysics->GetBodyInterface().SetLinearVelocity(m_bodyId, to_Jolt(v));
}
glm::vec3 RigidBody::getLinearVelocity() const {
  return to_glm(m_joltPhysics->GetBodyInterface().GetLinearVelocity(m_bodyId));
}

void RigidBody::applyImpulse(const glm::vec3 &v) {
  m_joltPhysics->GetBodyInterface().AddImpulse(m_bodyId, to_Jolt(v));
}
