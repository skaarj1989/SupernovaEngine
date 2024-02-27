find_package(spdlog REQUIRED)

add_library(AudioWorld
  "include/AudioWorld.hpp"
  "src/AudioWorld.cpp"
  "include/SoundSettings.hpp"
  "include/SoundSourceComponent.hpp"
  "src/SoundSourceComponent.cpp"
  "include/AudioPlayerComponent.hpp"
  "src/AudioPlayerComponent.cpp"

  "include/AudioClipResource.hpp"
  "src/AudioClipResource.cpp"
  "include/AudioClipLoader.hpp"
  "src/AudioClipLoader.cpp"
  "include/AudioClipManager.hpp"
  "src/AudioClipManager.cpp"
)
target_include_directories(AudioWorld PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(AudioWorld
  PUBLIC Resource FileSystem AudioDevice Transform
  PRIVATE spdlog::spdlog WaveDecoder VorbisDecoder
)
set_target_properties(AudioWorld PROPERTIES FOLDER "Framework/Audio")

enable_profiler(AudioWorld PRIVATE)
