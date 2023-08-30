add_library(StringUtility
  "include/StringHash.hpp"
  "include/StringUtility.hpp"
  "src/StringUtility.cpp"
)
target_include_directories(StringUtility PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
set_target_properties(StringUtility PROPERTIES FOLDER "Framework/")
