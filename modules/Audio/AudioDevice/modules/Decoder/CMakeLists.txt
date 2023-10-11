add_library(AudioDecoder INTERFACE "Decoder.hpp" "ClipInfo.hpp")
target_include_directories(AudioDecoder INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(AudioDecoder INTERFACE FileSystem)

set_target_properties(AudioDecoder PROPERTIES FOLDER "Framework/Audio")
