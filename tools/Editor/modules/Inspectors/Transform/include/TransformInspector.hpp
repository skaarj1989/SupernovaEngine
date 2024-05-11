#pragma once

#include "glm/fwd.hpp"

class Transform;

bool inspect(Transform &);
bool inspectPosition(glm::vec3 &);
bool inspectOrientation(glm::quat &);
bool inspectScale(glm::vec3 &);
