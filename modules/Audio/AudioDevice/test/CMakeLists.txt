find_package(Catch2 REQUIRED)

add_executable(TestAudio "TestAudio.cpp")
target_link_libraries(TestAudio
  PRIVATE Catch2::Catch2 AudioDevice VorbisDecoder WaveDecoder
)
set_target_properties(TestAudio PROPERTIES FOLDER "Tests")
