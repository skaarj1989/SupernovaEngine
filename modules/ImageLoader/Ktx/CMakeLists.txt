find_package(Ktx CONFIG REQUIRED)

add_library(KTXLoader "include/KTXLoader.hpp" "src/KTXLoader.cpp")
target_include_directories(KTXLoader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(KTXLoader
  PRIVATE FileSystem KTX::ktx
  PUBLIC VulkanRHI
)
set_target_properties(KTXLoader PROPERTIES FOLDER "Framework")
