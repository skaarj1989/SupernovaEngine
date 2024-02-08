#include "physics/Character.hpp"
#include "physics/Conversion.hpp"

Character::Character(const Settings &settings) : m_settings{settings} {}
Character::Character(const Character &other) : Character{other.m_settings} {}

Character::operator bool() const {
  return m_character && !m_character->GetBodyID().IsInvalid();
}

const Character::Settings &Character::getSettings() const { return m_settings; }

JPH::BodyID Character::getBodyId() const { return m_character->GetBodyID(); }

#define SETTER(Func, Value) m_character->Set##Func(to_Jolt(Value))
#define GETTER(Func) to_glm(m_character->Get##Func())

void Character::setRotation(const glm::quat &q) { SETTER(Rotation, q); }
void Character::setLinearVelocity(const glm::vec3 &v) {
  SETTER(LinearVelocity, v);
}

glm::vec3 Character::getPosition() const { return GETTER(Position); }
glm::quat Character::getRotation() const { return GETTER(Rotation); }
glm::vec3 Character::getLinearVelocity() const {
  return GETTER(LinearVelocity);
}

JPH::Character::EGroundState Character::getGroundState() const {
  return m_character->GetGroundState();
}
bool Character::isSupported() const { return m_character->IsSupported(); }
glm::vec3 Character::getGroundNormal() const { return GETTER(GroundNormal); }
glm::vec3 Character::getGroundVelocity() const {
  return GETTER(GroundVelocity);
}
