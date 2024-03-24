#include "glm2ai.hpp"
#include "glm/gtc/type_ptr.hpp" // value_ptr
#include "assimp/matrix4x4.inl"

aiMatrix4x4 to_mat4(const glm::mat4 &m) {
  static_assert(sizeof(std::decay_t<decltype(m)>) == sizeof(aiMatrix4x4));

  aiMatrix4x4 result;
  std::memcpy(&result.a1, glm::value_ptr(glm::transpose(m)),
              sizeof(aiMatrix4x4));
  return result;
}
