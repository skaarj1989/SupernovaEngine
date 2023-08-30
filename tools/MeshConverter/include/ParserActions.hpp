#pragma once

#include "argparse/argparse.hpp"
#include "glm/ext/quaternion_float.hpp"

[[nodiscard]] glm::vec3 parseVec3(const std::string_view);
[[nodiscard]] glm::quat parseQuat(const std::string_view);

[[nodiscard]] std::optional<glm::mat4>
getTransform(const argparse::ArgumentParser &);
