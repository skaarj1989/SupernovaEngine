add_library(SerializationUserDataAdapter INTERFACE "include/SerializationUserDataAdapter.hpp")
target_include_directories(SerializationUserDataAdapter
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include
)
set_target_properties(SerializationUserDataAdapter PROPERTIES FOLDER "Framework")
