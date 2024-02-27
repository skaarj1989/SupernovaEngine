find_package(spdlog REQUIRED)

add_library(ShaderCodeBuilder
  "include/ShaderCodeBuilder.hpp"
  "include/ShaderCodeBuilder.inl"
  "src/ShaderCodeBuilder.cpp"
)
target_include_directories(ShaderCodeBuilder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ShaderCodeBuilder PRIVATE spdlog::spdlog FileSystem)
set_target_properties(ShaderCodeBuilder PROPERTIES FOLDER "Framework")

enable_profiler(ShaderCodeBuilder PRIVATE)
