find_package(ZLIB REQUIRED)
find_package(unofficial-minizip CONFIG REQUIRED)

add_library(FileSystem
  "include/os/FileSystem.hpp"
  "src/FileSystem.cpp"

  "include/os/DataStream.hpp"
  "src/MemoryFile.hpp"
  "src/MemoryFile.cpp"
  "src/File.cpp"
  "src/File.hpp"

  "include/os/ZipArchive.hpp"
  "src/ZipArchive.cpp"
  "src/InZipFile.hpp"
  "src/InZipFile.cpp"

  "include/os/Serialization.hpp"
)
target_include_directories(FileSystem PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(FileSystem
  PRIVATE unofficial::minizip::minizip
  PUBLIC StringUtility
)
set_target_properties(FileSystem PROPERTIES FOLDER "Framework/OS")

enable_profiler(FileSystem PRIVATE)
