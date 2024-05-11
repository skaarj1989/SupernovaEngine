find_package(Catch2 CONFIG REQUIRED)

add_executable(TestDataTypeConversion "TestDataTypeConversion.cpp")
target_link_libraries(TestDataTypeConversion PRIVATE Catch2::Catch2 MaterialEditor)

add_executable(TestShaderCodeComposer "TestShaderCodeComposer.cpp")
target_link_libraries(TestShaderCodeComposer PRIVATE Catch2::Catch2 MaterialEditor)

include(CTest)
include(Catch)
catch_discover_tests(TestDataTypeConversion)
catch_discover_tests(TestShaderCodeComposer)

set_target_properties(
  TestDataTypeConversion TestShaderCodeComposer
  PROPERTIES FOLDER "Tests"
)
