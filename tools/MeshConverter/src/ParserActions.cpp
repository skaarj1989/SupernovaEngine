#include "ParserActions.hpp"
#include "glm/gtx/transform.hpp"  // translate, scale
#include "glm/gtx/quaternion.hpp" // toMat4

namespace {

std::istream &operator>>(std::istream &is, glm::vec3 &v) {
  char ignore;
  is >> v.x >> ignore >> v.y >> ignore >> v.z;
  return is;
}
std::istream &operator>>(std::istream &is, glm::quat &q) {
  char ignore;
  is >> q.x >> ignore >> q.y >> ignore >> q.z >> ignore >> q.w;
  return is;
}

[[nodiscard]] glm::quat rotate(const glm::vec3 &angles) {
  auto p = glm::angleAxis(glm::radians(angles.x), glm::vec3{1.0f, 0.0f, 0.0f});
  auto y = glm::angleAxis(glm::radians(angles.y), glm::vec3{0.0f, 1.0f, 0.0f});
  auto r = glm::angleAxis(glm::radians(angles.z), glm::vec3{0.0f, 0.0f, 1.0f});
  return glm::normalize(p * y * r);
}

} // namespace

glm::vec3 parseVec3(const std::string_view str) {
  std::istringstream ss{str.data()};
  glm::vec3 rv{};
  ss >> rv;
  return rv;
}
glm::quat parseQuat(const std::string_view str) {
  std::istringstream ss{str.data()};
  glm::quat rv{};
  ss >> rv;
  return rv;
}

std::optional<glm::mat4> getTransform(const argparse::ArgumentParser &args) {
#define T(v) (v ? glm::translate(*v) : glm::mat4{1.0f})
#define R(v) (v ? glm::toMat4(rotate(*v)) : glm::mat4{1.0f})
#define S(v) (v ? glm::scale(*v) : glm::mat4{1.0f})

  auto scale = args.present<glm::vec3>("--scale");
  auto eulerAngles = args.present<glm::vec3>("--rotate");
  auto position = args.present<glm::vec3>("--translate");
  if (scale || eulerAngles || position)
    return T(position) * R(eulerAngles) * S(scale);

#undef T
#undef R
#undef S

  return std::nullopt;
}
