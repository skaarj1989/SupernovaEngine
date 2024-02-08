#include "physics/RigidBody.hpp"
#include "physics/Conversion.hpp"

RigidBody::RigidBody(const Settings &settings) : m_settings{settings} {}
RigidBody::RigidBody(const RigidBody &other) : RigidBody{other.m_settings} {}

RigidBody::operator bool() const {
  return m_joltPhysics && !m_bodyId.IsInvalid();
}

const RigidBody::Settings &RigidBody::getSettings() const { return m_settings; }

JPH::BodyID RigidBody::getBodyId() const { return m_bodyId; }

#define BODY_INTERFACE m_joltPhysics->GetBodyInterface()
#define MUTATOR(Func, Value) BODY_INTERFACE.Func(m_bodyId, to_Jolt(Value))
#define MUTATOR_EX(Func, Value, ...)                                           \
  BODY_INTERFACE.Func(m_bodyId, to_Jolt(Value), __VA_ARGS__)
#define GETTER(Func) to_glm(BODY_INTERFACE.Get##Func(m_bodyId))

void RigidBody::setPosition(const glm::vec3 &v) {
  MUTATOR_EX(SetPosition, v, JPH::EActivation::DontActivate);
}
void RigidBody::setRotation(const glm::quat &q) {
  MUTATOR_EX(SetRotation, q, JPH::EActivation::DontActivate);
}
void RigidBody::setLinearVelocity(const glm::vec3 &v) {
  MUTATOR(SetLinearVelocity, v);
}

void RigidBody::applyImpulse(const glm::vec3 &v) { MUTATOR(AddImpulse, v); }

glm::vec3 RigidBody::getPosition() const { return GETTER(Position); }
glm::quat RigidBody::getRotation() const { return GETTER(Rotation); }
glm::vec3 RigidBody::getLinearVelocity() const {
  return GETTER(LinearVelocity);
}
