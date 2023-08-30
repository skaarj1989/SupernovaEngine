find_package(glm REQUIRED)

add_library(Math
  "include/math/Math.hpp"
  "include/math/Hash.hpp"
  "include/math/Color.hpp"
  "include/math/CollisionDetection.hpp"
  "src/CollisionDetection.cpp"
  "include/math/Frustum.hpp"
  "src/Frustum.cpp"
  "include/math/AABB.hpp"
  "src/AABB.cpp"
  "include/math/Cone.hpp"
  "include/math/Sphere.hpp"
  "include/math/Plane.hpp"
  "src/Plane.cpp"

  "include/math/Serialization.hpp"
  "include/math/json.hpp"
)
target_compile_definitions(Math PUBLIC GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_include_directories(Math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(Math PUBLIC glm::glm)
set_target_properties(Math PROPERTIES FOLDER "Framework")
