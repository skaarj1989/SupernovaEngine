#pragma once

#include <optional>
#include <string>
#include <utility>
#include <cstdint>

// clang-format off
enum class DataType {
  Undefined = 0,

  Bool, BVec2, BVec3, BVec4,
  Int32, IVec2, IVec3, IVec4,
  UInt32, UVec2, UVec3, UVec4,
  Float, Vec2, Vec3, Vec4,
  Double, DVec2, DVec3, DVec4,

  Mat2, Mat3, Mat4,

  Sampler2D, SamplerCube,
};
// clang-format on

[[nodiscard]] constexpr auto makeHash(DataType lhs, DataType rhs) {
  return std::to_underlying(lhs) << 16 | std::to_underlying(rhs);
}

[[nodiscard]] constexpr auto isScalar(DataType type) {
  switch (type) {
    using enum DataType;

  case Bool:
  case Int32:
  case UInt32:
  case Float:
  case Double:
    return true;
  }
  return false;
}
[[nodiscard]] constexpr auto isVector(DataType type) {
  switch (type) {
    using enum DataType;

  case BVec2:
  case BVec3:
  case BVec4:

  case IVec2:
  case IVec3:
  case IVec4:

  case UVec2:
  case UVec3:
  case UVec4:

  case Vec2:
  case Vec3:
  case Vec4:

  case DVec2:
  case DVec3:
  case DVec4:
    return true;
  }
  return false;
}
[[nodiscard]] constexpr auto isMatrix(DataType type) {
  switch (type) {
    using enum DataType;

  case Mat2:
  case Mat3:
  case Mat4:
    return true;
  }
  return false;
}
[[nodiscard]] constexpr auto isSampler(DataType type) {
  switch (type) {
    using enum DataType;

  case Sampler2D:
  case SamplerCube:
    return true;
  }
  return false;
}

[[nodiscard]] constexpr auto getBaseDataType(DataType type) {
  using enum DataType;
  switch (type) {
  case Bool:
  case Int32:
  case UInt32:
  case Float:
  case Double:
    return type;

  case BVec2:
  case BVec3:
  case BVec4:
    return Bool;
  case IVec2:
  case IVec3:
  case IVec4:
    return Int32;
  case UVec2:
  case UVec3:
  case UVec4:
    return UInt32;
  case Vec2:
  case Vec3:
  case Vec4:
    return Float;
  case DVec2:
  case DVec3:
  case DVec4:
    return Double;

  case Mat2:
  case Mat3:
  case Mat4:
    return Float;
  }
  return Undefined;
}

[[nodiscard]] constexpr auto countChannels(DataType type) {
  switch (type) {
    using enum DataType;

  case Bool:
  case Int32:
  case UInt32:
  case Float:
  case Double:
    return 1;

  case BVec2:
  case IVec2:
  case UVec2:
  case Vec2:
  case DVec2:
    return 2;

  case BVec3:
  case IVec3:
  case UVec3:
  case Vec3:
  case DVec3:
    return 3;

  case BVec4:
  case IVec4:
  case UVec4:
  case Vec4:
  case DVec4:
    return 4;
  }
  return 0;
}
[[nodiscard]] constexpr auto countColumns(DataType type) {
  switch (type) {
    using enum DataType;

  case Mat2:
    return 2;
  case Mat3:
    return 3;
  case Mat4:
    return 4;
  }
  return 0;
}

[[nodiscard]] float calcOptimalInspectorWidth(DataType);

[[nodiscard]] const char *toString(DataType);

// ---

[[nodiscard]] DataType constructVectorType(DataType baseType,
                                           uint32_t numChannels);

[[nodiscard]] std::optional<std::string_view> getConversionFormat(DataType from,
                                                                  DataType to);
