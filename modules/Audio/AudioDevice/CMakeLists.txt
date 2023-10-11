find_package(glm REQUIRED)
find_package(OpenAL CONFIG REQUIRED)

add_subdirectory(modules/Decoder)

add_library(AudioDevice
  "include/audio/Device.hpp"
  "src/Device.cpp"
  "include/audio/Buffer.hpp"
  "src/Buffer.cpp"
  "include/audio/SourceBase.hpp"
  "src/SourceBase.cpp"
  "include/audio/Source.hpp"
  "src/Source.cpp"
  "include/audio/StreamPlayer.hpp"
  "src/StreamPlayer.cpp"
  "src/ALHelper.hpp"
  "src/ALHelper.cpp"
  "src/ALCheck.hpp"
  "include/audio/Utility.hpp"
)

target_include_directories(AudioDevice
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(AudioDevice
  PUBLIC glm::glm OpenAL::OpenAL AudioDecoder
)
set_target_properties(AudioDevice PROPERTIES FOLDER "Framework/Audio")

add_subdirectory(modules)

add_subdirectory(test)
