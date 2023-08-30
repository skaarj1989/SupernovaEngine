add_library(Nuklear "include/Nuklear/nuklear.h" "src/nuklear.cpp")
target_compile_definitions(Nuklear
    PUBLIC NK_INCLUDE_DEFAULT_FONT NK_INCLUDE_FONT_BAKING
)
target_include_directories(Nuklear PUBLIC include/)
