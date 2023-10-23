FetchContent_Declare(glad
  GIT_REPOSITORY https://github.com/skaarj1989/glad.git
  GIT_TAG vk
)
FetchContent_MakeAvailable(glad)
set_target_properties(glad PROPERTIES FOLDER "Third-party")

add_subdirectory(VulkanMemoryAllocator)
