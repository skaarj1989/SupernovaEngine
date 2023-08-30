#pragma once

#include "catch.hpp"

#include "MaterialEditor/ShaderCodeComposer.hpp"
#include "rhi/ShaderCompiler.hpp"

// https://stackoverflow.com/questions/63823544/default-construct-all-the-types-in-a-stdvariant-and-put-them-in-a-stdvector

template <std::size_t... Idx>
void addValues(ShaderCodeComposer &composer, std::index_sequence<Idx...>) {
  (composer.addVariable(std::format("id_{}", Idx),
                        std::variant_alternative_t<Idx, ValueVariant>{1}),
   ...);
}

SCENARIO("GLSL Compiles") {
  ShaderCodeComposer composer;
  addValues(composer,
            std::make_index_sequence<std::variant_size_v<ValueVariant>>{});

  // clang-format off
  const auto code = std::format(R"(
#version 460

void main() {{
{}
}})", composer.getCode());
  // clang-format on

  REQUIRE(rhi::ShaderCompiler{}.compile(rhi::ShaderType::Vertex, code));
}

int main(int argc, char *argv[]) { return Catch::Session{}.run(argc, argv); }
