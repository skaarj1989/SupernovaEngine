find_package(Catch2 CONFIG REQUIRED)

add_executable(TestDataTypeConversion
    "../src/MaterialEditor/DataType.cpp"
    "TestDataTypeConversion.cpp"
)
target_include_directories(TestDataTypeConversion PUBLIC ../include)
target_link_libraries(TestDataTypeConversion PRIVATE Catch2::Catch2 VulkanRHI)

add_executable(TestShaderCodeComposer
    "../src/MaterialEditor/DataType.cpp"
    "../src/MaterialEditor/ValueVariant.cpp"
    "../src/MaterialEditor/ShaderCodeComposer.cpp"
    "TestShaderCodeComposer.cpp"
)
target_include_directories(TestShaderCodeComposer PUBLIC ../include)
target_link_libraries(TestShaderCodeComposer
    PRIVATE Catch2::Catch2 StringUtility SerializationUserDataAdapter WorldRenderer
)

include(CTest)
include(Catch)
catch_discover_tests(TestDataTypeConversion)
catch_discover_tests(TestShaderCodeComposer)

set_target_properties(
    TestDataTypeConversion TestShaderCodeComposer
    PROPERTIES FOLDER "Tests"
)
