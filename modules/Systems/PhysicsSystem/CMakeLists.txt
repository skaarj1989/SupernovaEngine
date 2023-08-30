find_package(spdlog REQUIRED)

add_library(PhysicsSystem "include/PhysicsSystem.hpp" "src/PhysicsSystem.cpp")
target_include_directories(PhysicsSystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(PhysicsSystem
  PRIVATE spdlog::spdlog
  PUBLIC SystemCommon JoltPhysics
)
set_target_properties(PhysicsSystem PROPERTIES FOLDER "Framework/Systems")
