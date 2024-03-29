#pragma once

#include "renderer/Vertex1p1n1st.hpp"
#include "glm/ext/scalar_constants.inl" // pi
#include "glm/trigonometric.hpp"        // sin, cos

// http://www.songho.ca/opengl/gl_sphere.html
// https://www.danielsieger.com/blog/2021/05/03/generating-primitive-shapes.html

namespace gfx {

struct GeometryData {
  std::vector<Vertex1p1n1st> vertices;
  std::vector<uint16_t> indices;
};

[[nodiscard]] auto buildPlane(float size, float tiling = 1.0f) {
  constexpr glm::vec3 kUp{0.0f, 1.0f, 0.0f};
  return std::vector{
    // clang-format off
    Vertex1p1n1st
    {glm::vec3{ 1.0f, 0.0f, 1.0f} * size, kUp, glm::vec2{1.0f, 1.0f} * tiling},
    {glm::vec3{-1.0f, 0.0f,-1.0f} * size, kUp, glm::vec2{0.0f, 0.0f} * tiling},
    {glm::vec3{-1.0f, 0.0f, 1.0f} * size, kUp, glm::vec2{0.0f, 1.0f} * tiling},

    {glm::vec3{ 1.0f, 0.0f, 1.0f} * size, kUp, glm::vec2{1.0f, 1.0f} * tiling},
    {glm::vec3{ 1.0f, 0.0f,-1.0f} * size, kUp, glm::vec2{1.0f, 0.0f} * tiling},
    {glm::vec3{-1.0f, 0.0f,-1.0f} * size, kUp, glm::vec2{0.0f, 0.0f} * tiling}
    // clang-format on
  };
}

template <uint8_t _Width, uint8_t _Height>
[[nodiscard]] auto buildSubdividedPlane() {
  constexpr auto offset = glm::vec3{-(_Height - 1), 0.0f, -(_Width - 1)} * 0.5f;
  constexpr auto texelSize = 1.0f / glm::vec2{_Width - 1, _Height - 1};

  constexpr auto kNumVertices = _Width * _Height * 3;
  std::vector<Vertex1p1n1st> vertices(kNumVertices);

  auto i = 0;
  for (decltype(_Height) row = 0; row < _Height; ++row) {
    for (decltype(_Width) col = 0; col < _Width; ++col) {
      constexpr auto kUp = glm::vec3{0.0f, 1.0f, 0.0f};
      vertices[i++] = Vertex1p1n1st{
        .position = offset + glm::vec3{col, 0.0f, row},
        .normal = kUp,
        .texCoord = glm::vec2{col, row} * texelSize,
      };
    }
  }

  constexpr auto kNumStripes = _Height - 1;
  constexpr auto kNumIndices = kNumStripes * (2 * _Width - 1) + 1;
  std::vector<uint16_t> indices(kNumIndices);

  i = 0;
  for (decltype(_Height) row = 0; row < _Height - 1; ++row) {
    if ((row & 1) == 0) {
      // Even rows.
      for (decltype(_Width) col = 0; col < _Width; ++col) {
        indices[i++] = col + row * _Width;
        indices[i++] = col + (row + 1) * _Width;
      }
    } else {
      // Odd rows.
      for (decltype(_Width) col = _Width - 1; col > 0; --col) {
        indices[i++] = col + (row + 1) * _Width;
        indices[i++] = col - 1 + +row * _Width;
      }
    }
  }
  if ((_Height & 1) && _Height > 2) {
    indices[i++] = (_Height - 1) * _Width;
  }

  return GeometryData{
    .vertices = std::move(vertices),
    .indices = std::move(indices),
  };
}

[[nodiscard]] auto buildCube(float size) {
  constexpr glm::vec3 kX{1.0f, 0.0f, 0.0f};
  constexpr glm::vec3 kY{0.0f, 1.0f, 0.0f};
  constexpr glm::vec3 kZ{0.0f, 0.0f, 1.0f};

  // clang-format off
  return std::vector<Vertex1p1n1st>{
    // -X
    {glm::vec3{-1.0f,-1.0f,-1.0f} * size, -kX, {0.0f, 1.0f}},
    {glm::vec3{-1.0f,-1.0f, 1.0f} * size, -kX, {1.0f, 1.0f}},
    {glm::vec3{-1.0f, 1.0f, 1.0f} * size, -kX, {1.0f, 0.0f}},
    {glm::vec3{-1.0f, 1.0f, 1.0f} * size, -kX, {1.0f, 0.0f}},
    {glm::vec3{-1.0f, 1.0f,-1.0f} * size, -kX, {0.0f, 0.0f}},
    {glm::vec3{-1.0f,-1.0f,-1.0f} * size, -kX, {0.0f, 1.0f}},

    // -Z
    {glm::vec3{-1.0f,-1.0f,-1.0f} * size, -kZ, {1.0f, 1.0f}},
    {glm::vec3{ 1.0f, 1.0f,-1.0f} * size, -kZ, {0.0f, 0.0f}},
    {glm::vec3{ 1.0f,-1.0f,-1.0f} * size, -kZ, {0.0f, 1.0f}},
    {glm::vec3{-1.0f,-1.0f,-1.0f} * size, -kZ, {1.0f, 1.0f}},
    {glm::vec3{-1.0f, 1.0f,-1.0f} * size, -kZ, {1.0f, 0.0f}},
    {glm::vec3{ 1.0f, 1.0f,-1.0f} * size, -kZ, {0.0f, 0.0f}},

    // -Y
    {glm::vec3{-1.0f,-1.0f,-1.0f} * size, -kY, {1.0f, 0.0f}},
    {glm::vec3{ 1.0f,-1.0f,-1.0f} * size, -kY, {1.0f, 1.0f}},
    {glm::vec3{ 1.0f,-1.0f, 1.0f} * size, -kY, {0.0f, 1.0f}},
    {glm::vec3{-1.0f,-1.0f,-1.0f} * size, -kY, {1.0f, 0.0f}},
    {glm::vec3{ 1.0f,-1.0f, 1.0f} * size, -kY, {0.0f, 1.0f}},
    {glm::vec3{-1.0f,-1.0f, 1.0f} * size, -kY, {0.0f, 0.0f}},

    // +Y
    {glm::vec3{-1.0f, 1.0f,-1.0f} * size, kY, {1.0f, 0.0f}},
    {glm::vec3{-1.0f, 1.0f, 1.0f} * size, kY, {0.0f, 0.0f}},
    {glm::vec3{ 1.0f, 1.0f, 1.0f} * size, kY, {0.0f, 1.0f}},
    {glm::vec3{-1.0f, 1.0f,-1.0f} * size, kY, {1.0f, 0.0f}},
    {glm::vec3{ 1.0f, 1.0f, 1.0f} * size, kY, {0.0f, 1.0f}},
    {glm::vec3{ 1.0f, 1.0f,-1.0f} * size, kY, {1.0f, 1.0f}},

    // +X
    {glm::vec3{1.0f, 1.0f,-1.0f} * size, kX, {1.0f, 0.0f}},
    {glm::vec3{1.0f, 1.0f, 1.0f} * size, kX, {0.0f, 0.0f}},
    {glm::vec3{1.0f,-1.0f, 1.0f} * size, kX, {0.0f, 1.0f}},
    {glm::vec3{1.0f,-1.0f, 1.0f} * size, kX, {0.0f, 1.0f}},
    {glm::vec3{1.0f,-1.0f,-1.0f} * size, kX, {1.0f, 1.0f}},
    {glm::vec3{1.0f, 1.0f,-1.0f} * size, kX, {1.0f, 0.0f}},

    // +Z
    {glm::vec3{-1.0f, 1.0f, 1.0f} * size, kZ, {0.0f, 0.0f}},
    {glm::vec3{-1.0f,-1.0f, 1.0f} * size, kZ, {0.0f, 1.0f}},
    {glm::vec3{ 1.0f, 1.0f, 1.0f} * size, kZ, {1.0f, 0.0f}},
    {glm::vec3{-1.0f,-1.0f, 1.0f} * size, kZ, {0.0f, 1.0f}},
    {glm::vec3{ 1.0f,-1.0f, 1.0f} * size, kZ, {1.0f, 1.0f}},
    {glm::vec3{ 1.0f, 1.0f, 1.0f} * size, kZ, {1.0f, 0.0f}},
  };
  // clang-format on
}

template <uint8_t _NumSectors = 36, uint8_t _NumStacks = 18>
[[nodiscard]] auto buildSphere(float radius) {
  // http://www.songho.ca/opengl/gl_sphere.html

  static_assert(_NumSectors >= 3 && _NumStacks >= 2);

  constexpr auto kNumVertices = (_NumSectors + 1) * (_NumStacks + 1);
  std::vector<Vertex1p1n1st> vertices(kNumVertices);

  constexpr auto kPI = glm::pi<float>();
  constexpr auto kSectorStep = 2 * kPI / _NumSectors;
  constexpr auto kStackStep = kPI / _NumStacks;
  const auto invLength = 1.0f / radius;

  auto i = 0;

  Vertex1p1n1st vertex{};
  for (decltype(_NumStacks) stack = 0; stack <= _NumStacks; ++stack) {
    // Starting from pi / 2 to - pi / 2

    const float stackAngle{kPI / 2.0f - stack * kStackStep};
    const auto xy = radius * glm::cos(stackAngle);
    vertex.position.z = radius * glm::sin(stackAngle);

    // Add (sectorCount + 1) vertices per stack.
    // The first and last vertices have same position and normal, but different
    // texCoords.
    for (decltype(_NumSectors) sector = 0; sector <= _NumSectors; ++sector) {
      const float sectorAngle = sector * kSectorStep; // Starting from 0 to 2pi.

      vertex.position.x = xy * glm::cos(sectorAngle); // r * cos(u) * cos(v)
      vertex.position.y = xy * glm::sin(sectorAngle); // r * cos(u) * sin(v)

      vertex.normal = vertex.position * invLength;

      vertex.texCoord = {
        float(sector) / _NumSectors,
        float(stack) / _NumStacks,
      };
      vertices[i++] = vertex;
    }
  }

  constexpr auto kNumIndices = _NumSectors * (_NumStacks - 1) * 6;
  std::vector<uint16_t> indices(kNumIndices);
  i = 0;

  //  k1--k1+1
  //  |  / |
  //  | /  |
  //  k2--k2+1
  for (decltype(_NumStacks) stack = 0; stack < _NumStacks; ++stack) {
    // Beginning of current stack.
    auto k1 = static_cast<uint16_t>(stack * (_NumSectors + 1u));
    // Beginning of next stack.
    auto k2 = static_cast<uint16_t>(k1 + _NumSectors + 1u);

    for (decltype(_NumSectors) sector = 0; sector < _NumSectors;
         ++sector, ++k1, ++k2) {
      // 2 triangles per sector excluding 1st and last stacks.
      if (stack != 0) {
        indices[i++] = k1;
        indices[i++] = k2;
        indices[i++] = k1 + 1u;
      }
      if (stack != (_NumStacks - 1)) {
        indices[i++] = k1 + 1u;
        indices[i++] = k2;
        indices[i++] = k2 + 1u;
      }
    }
  }

  return GeometryData{
    .vertices = std::move(vertices),
    .indices = std::move(indices),
  };
}

} // namespace gfx
