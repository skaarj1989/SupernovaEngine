#pragma once

#include "glm/ext/matrix_float4x4.hpp"
#include "assimp/matrix4x4.h"

[[nodiscard]] aiMatrix4x4 to_mat4(const glm::mat4 &);
