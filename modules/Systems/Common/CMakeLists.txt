find_package(EnTT REQUIRED)
find_package(cereal REQUIRED)

add_library(SystemCommon INTERFACE "include/SystemSerializationContext.hpp" "include/SystemCommons.hpp")
target_include_directories(SystemCommon INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(SystemCommon INTERFACE EnTT::EnTT cereal::cereal)
set_target_properties(SystemCommon PROPERTIES FOLDER "Framework/Systems")
