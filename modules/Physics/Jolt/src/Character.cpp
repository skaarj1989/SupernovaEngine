#include "physics/Character.hpp"
#include "physics/Conversion.hpp"

Character::Character(const CharacterSettings &settings)
    : m_settings{settings} {}

void Character::setCollisionShape(const JPH::Shape *shape) {
  assert(shape);
  m_character->SetShape(shape, FLT_MAX);
}

const CharacterSettings &Character::getSettings() const { return m_settings; }

glm::quat Character::getRotation() const {
  return to_glm(m_character->GetRotation());
}
void Character::setRotation(const glm::quat &q) {
  m_character->SetRotation(to_Jolt(q));
}

void Character::setLinearVelocity(const glm::vec3 &v) {
  m_character->SetLinearVelocity(to_Jolt(v));
}
glm::vec3 Character::getLinearVelocity() const {
  return to_glm(m_character->GetLinearVelocity());
}

JPH::Character::EGroundState Character::getGroundState() const {
  return m_character->GetGroundState();
}
glm::vec3 Character::getGroundNormal() const {
  return to_glm(m_character->GetGroundNormal());
}
