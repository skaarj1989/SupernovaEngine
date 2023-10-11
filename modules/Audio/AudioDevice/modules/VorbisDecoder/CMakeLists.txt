find_package(Ogg CONFIG REQUIRED)
find_package(Vorbis CONFIG REQUIRED)

add_library(VorbisDecoder "VorbisDecoder.hpp" "VorbisDecoder.cpp")
target_include_directories(VorbisDecoder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(VorbisDecoder
  PUBLIC AudioDecoder
  PRIVATE Ogg::ogg Vorbis::vorbis Vorbis::vorbisenc Vorbis::vorbisfile
)
set_target_properties(VorbisDecoder PROPERTIES FOLDER "Framework/Audio/Decoders")
