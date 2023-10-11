add_library(WaveDecoder "WaveDecoder.hpp" "WaveDecoder.cpp")
target_include_directories(WaveDecoder PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(WaveDecoder PUBLIC AudioDecoder)
set_target_properties(WaveDecoder PROPERTIES FOLDER "Framework/Audio/Decoders")
