find_package(spdlog REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(argparse CONFIG REQUIRED)

find_package(meshoptimizer CONFIG REQUIRED)

FetchContent_Declare(assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG master
)

set(ASSIMP_INSTALL OFF CACHE BOOL "")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")

set(ASSIMP_RAPIDJSON_NO_MEMBER_ITERATOR OFF CACHE BOOL "")
set(ASSIMP_BUILD_NO_EXPORT OFF CACHE BOOL "")
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT FALSE CACHE BOOL "")

set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "")

FetchContent_MakeAvailable(assimp)

if (MSVC)
  set_target_properties(assimp zlibstatic UpdateAssimpLibsDebugSymbolsAndDLLs
    PROPERTIES FOLDER "Third-party/assimp"
  )
endif()

add_executable(MeshConverter
  "src/main.cpp"

  "include/ParserActions.hpp"
  "src/ParserActions.cpp"

  "include/PrintSceneStats.hpp"
  "src/PrintSceneStats.cpp"

  "include/ExtraAnimations.hpp"
  "src/ExtraAnimations.cpp"

  "include/SupernovaMeshExport.hpp"
  "src/SupernovaMeshExport.cpp"

  "include/ai2glm.hpp"
  "src/ai2glm.cpp"
  "src/ai2ozz.hpp"
  "src/ai2ozz.cpp"
  "src/glm2ai.hpp"
  "src/glm2ai.cpp"
  "src/aiStringHash.hpp"

  "src/MaterialConverter.hpp"
  "src/MaterialConverter.cpp"

  "src/MaterialDebugInfo.hpp"
  "src/MaterialDebugInfo.cpp"

  "src/AnimationConverter.hpp"
  "src/AnimationConverter.cpp"

  "src/MeshExporter.hpp"
  "src/MeshExporter.cpp"

  "src/Mesh.hpp"
  "src/Mesh.cpp"
  "src/SubMesh.hpp"
  "src/SubMesh.cpp"
  "src/Material.hpp"
  "src/VertexInfo.hpp"
  "src/VertexInfo.cpp"
  "src/VertexBuffer.hpp"
  "src/VertexBuffer.cpp"
  "src/ByteBuffer.hpp"

  "src/Logger.hpp"
  "src/MakeSpan.hpp"
)
target_include_directories(MeshConverter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(MeshConverter
  PRIVATE
  spdlog::spdlog
  argparse::argparse
  Common
  Math
  WorldRenderer
  assimp::assimp
  meshoptimizer::meshoptimizer
  ozz_animation_offline
  nlohmann_json::nlohmann_json
)
set_target_properties(MeshConverter PROPERTIES FOLDER "Tools")
set_warning_level(MeshConverter)

set_debugger_working_directory(MeshConverter)
