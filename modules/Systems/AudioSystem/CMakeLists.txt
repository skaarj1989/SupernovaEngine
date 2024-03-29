find_package(spdlog REQUIRED)

add_library(AudioSystem
  "include/AudioSystem.hpp"
  "src/AudioSystem.cpp"
)
target_include_directories(AudioSystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(AudioSystem
  PUBLIC SystemCommon AudioWorld
  PRIVATE spdlog::spdlog
)
set_target_properties(AudioSystem PROPERTIES FOLDER "Framework/Systems")
set_warning_level(AudioSystem)
