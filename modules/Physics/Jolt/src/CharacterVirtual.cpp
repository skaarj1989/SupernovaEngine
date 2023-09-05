#include "physics/CharacterVirtual.hpp"
#include "physics/Conversion.hpp"

CharacterVirtual::CharacterVirtual(const Settings &settings)
    : m_settings{settings} {}

CharacterVirtual::operator bool() const { return m_character; }

const CharacterVirtual::Settings &CharacterVirtual::getSettings() const {
  return m_settings;
}

#define SETTER(Func, Value) m_character->Set##Func(to_Jolt(Value))
#define GETTER(Func) to_glm(m_character->Get##Func())

void CharacterVirtual::setRotation(const glm::quat &q) { SETTER(Rotation, q); }
void CharacterVirtual::setLinearVelocity(const glm::vec3 &v) {
  SETTER(LinearVelocity, v);
}

void CharacterVirtual::setStickToFloor(bool b) { m_stickToFloor = b; }
void CharacterVirtual::setWalkStairs(bool b) { m_walkStairs = b; }

glm::vec3 CharacterVirtual::getUp() const { return GETTER(Up); }

glm::vec3 CharacterVirtual::getPosition() const { return GETTER(Position); }
glm::quat CharacterVirtual::getRotation() const { return GETTER(Rotation); }
glm::vec3 CharacterVirtual::getLinearVelocity() const {
  return GETTER(LinearVelocity);
}

bool CharacterVirtual::isStickToFloor() const { return m_stickToFloor; }
bool CharacterVirtual::canWalkStairs() const { return m_walkStairs; }

JPH::CharacterVirtual::EGroundState CharacterVirtual::getGroundState() const {
  return m_character->GetGroundState();
}
bool CharacterVirtual::isSupported() const {
  return m_character->IsSupported();
}
glm::vec3 CharacterVirtual::getGroundNormal() const {
  return GETTER(GroundNormal);
}
glm::vec3 CharacterVirtual::getGroundVelocity() const {
  return GETTER(GroundVelocity);
}
