find_path(STB_INCLUDE_DIRS "stb_image.h")

add_library(STBImageLoader "include/STBImageLoader.hpp" "src/STBImageLoader.cpp")
target_link_libraries(STBImageLoader
  PRIVATE FileSystem
  PUBLIC VulkanRHI
)
target_include_directories(STBImageLoader
  PRIVATE ${STB_INCLUDE_DIRS}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)
set_target_properties(STBImageLoader PROPERTIES FOLDER "Framework")
