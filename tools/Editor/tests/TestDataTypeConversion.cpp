#pragma once

#include "catch.hpp"

#include "MaterialEditor/DataType.hpp"
#include "rhi/ShaderCompiler.hpp"

template <typename Func> void eachType(Func f) {
  using enum DataType;

  // clang-format off
  constexpr auto allTypes = {
    Bool, BVec2, BVec3, BVec4,
    UInt32, UVec2, UVec3, UVec4,
    Int32, IVec2, IVec3, IVec4,
    Float, Vec2, Vec3, Vec4,
    Double, DVec2, DVec3, DVec4,

    // Can't convert samplers.
  };
  // clang-format on

  for (const auto from : allTypes) {
    for (const auto to : allTypes)
      if (from != to) std::invoke(f, from, to);
  }
}

SCENARIO("Has all conversions") {
  eachType([](DataType from, DataType to) {
    INFO(std::format("{}->{}", toString(from), toString(to)));
    REQUIRE(getConversionFormat(from, to));
  });
}

SCENARIO("GLSL Compiles") {
  static const auto constructZero = [](DataType dataType) {
    return std::format("{}(0)", toString(dataType));
  };
  static const auto defineVar = [](DataType dataType, const std::string &rhs) {
    static auto i = 0;
    return std::format("const {} v{} = {};", toString(dataType), i++, rhs);
  };

  std::ostringstream oss;
  eachType([&oss](DataType from, DataType to) {
    if (const auto fmt = getConversionFormat(from, to); fmt) {
      const auto rhs =
        std::vformat(*fmt, std::make_format_args(constructZero(from)));
      std::ostream_iterator<std::string>{oss, "\n"} = defineVar(to, rhs);
    }
  });

  // clang-format off
  const auto code = std::format(R"(
#version 460

void main() {{
{}
}})", oss.str());
  // clang-format on

  REQUIRE(rhi::ShaderCompiler{}.compile(rhi::ShaderType::Vertex, code));
}

int main(int argc, char *argv[]) { return Catch::Session{}.run(argc, argv); }
