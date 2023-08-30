#include "MaterialEditor/DataType.hpp"
#include <cassert>

namespace {

[[nodiscard]] auto countWidgets(DataType dataType) {
  switch (dataType) {
    using enum DataType;

  case Bool:
  case UInt32:
  case Int32:
  case Float:
  case Double:
    return 1;

  case BVec2:
  case UVec2:
  case IVec2:
  case Vec2:
  case DVec2:
  case Mat2:
    return 2;

  case BVec3:
  case UVec3:
  case IVec3:
  case Vec3:
  case DVec3:
  case Mat3:
    return 3;

  case BVec4:
  case UVec4:
  case IVec4:
  case Vec4:
  case DVec4:
  case Mat4:
    return 4;
  }
  return 0;
}
[[nodiscard]] auto calcWidthPerWidget(DataType dataType) {
  switch (dataType) {
    using enum DataType;

  case UInt32:
  case Int32:
    return 10.0f * 3;

  case Float:
    // 2 = leading x.
    return 10.0f * (2 + 3);
  case Double:
    return 10.0f * (2 + 6);
  }
  return 0.0f;
}

} // namespace

float calcOptimalInspectorWidth(DataType dataType) {
  const auto baseType = getBaseDataType(dataType);
  return countWidgets(dataType) * calcWidthPerWidget(baseType);
}
const char *toString(DataType type) {
  // clang-format off
  switch (type) {
    using enum DataType;

  case Undefined: return "Undefined";

  case Bool: return "bool";
  case BVec2: return "bvec2";
  case BVec3: return "bvec3";
  case BVec4: return "bvec4";

  case UInt32: return "uint";
  case UVec2: return "uvec2";
  case UVec3: return "uvec3";
  case UVec4: return "uvec4";

  case Int32: return "int";
  case IVec2: return "ivec2";
  case IVec3: return "ivec3";
  case IVec4: return "ivec4";

  case Float: return "float";
  case Vec2: return "vec2";
  case Vec3: return "vec3";
  case Vec4: return "vec4";

  case Double: return "double";
  case DVec2: return "dvec2";
  case DVec3: return "dvec3";
  case DVec4: return "dvec4";

  case Mat2: return "mat2";
  case Mat3: return "mat3";
  case Mat4: return "mat4";

  case Sampler2D: return "sampler2D";
  case SamplerCube: return "samplerCube";
  }
  // clang-format on

  assert(false);
  return "Undefined";
}

DataType constructVectorType(DataType baseType, uint32_t numChannels) {
  assert(isScalar(baseType) && numChannels > 0 && numChannels <= 4);

  auto v = std::to_underlying(baseType);
  v += (numChannels - 1);
  return static_cast<DataType>(v);
}

std::optional<std::string_view> getConversionFormat(DataType from,
                                                    DataType to) {
  assert(from != to);

  switch (makeHash(from, to)) {
    using enum DataType;

#pragma region BasicTypes
#pragma region->bool
  case makeHash(UInt32, Bool):
  case makeHash(Int32, Bool):
  case makeHash(Float, Bool):
  case makeHash(Double, Bool):
    return "bool({})";
  case makeHash(UVec2, Bool):
  case makeHash(IVec2, Bool):
  case makeHash(Vec2, Bool):
  case makeHash(DVec2, Bool):
  case makeHash(UVec3, Bool):
  case makeHash(IVec3, Bool):
  case makeHash(Vec3, Bool):
  case makeHash(DVec3, Bool):
  case makeHash(UVec4, Bool):
  case makeHash(IVec4, Bool):
  case makeHash(Vec4, Bool):
  case makeHash(DVec4, Bool):
    return "bool({}.r)";
  case makeHash(BVec2, Bool):
  case makeHash(BVec3, Bool):
  case makeHash(BVec4, Bool):
    return "{}.r";
#pragma endregion->bool
#pragma region->uint
  case makeHash(Bool, UInt32):
  case makeHash(Int32, UInt32):
  case makeHash(Float, UInt32):
  case makeHash(Double, UInt32):
    return "uint({})";
  case makeHash(BVec2, UInt32):
  case makeHash(IVec2, UInt32):
  case makeHash(Vec2, UInt32):
  case makeHash(DVec2, UInt32):
  case makeHash(BVec3, UInt32):
  case makeHash(IVec3, UInt32):
  case makeHash(Vec3, UInt32):
  case makeHash(DVec3, UInt32):
  case makeHash(BVec4, UInt32):
  case makeHash(IVec4, UInt32):
  case makeHash(Vec4, UInt32):
  case makeHash(DVec4, UInt32):
    return "uint({}.r)";
  case makeHash(UVec2, UInt32):
  case makeHash(UVec3, UInt32):
  case makeHash(UVec4, UInt32):
    return "{}.r";
#pragma endregion->uint
#pragma region->int
  case makeHash(Bool, Int32):
  case makeHash(UInt32, Int32):
  case makeHash(Float, Int32):
  case makeHash(Double, Int32):
    return "int({})";
  case makeHash(BVec2, Int32):
  case makeHash(UVec2, Int32):
  case makeHash(Vec2, Int32):
  case makeHash(DVec2, Int32):
  case makeHash(BVec3, Int32):
  case makeHash(UVec3, Int32):
  case makeHash(Vec3, Int32):
  case makeHash(DVec3, Int32):
  case makeHash(BVec4, Int32):
  case makeHash(UVec4, Int32):
  case makeHash(Vec4, Int32):
  case makeHash(DVec4, Int32):
    return "int({}.r)";
  case makeHash(IVec2, Int32):
  case makeHash(IVec3, Int32):
  case makeHash(IVec4, Int32):
    return "{}.r";
#pragma endregion->int
#pragma region->float
  case makeHash(Bool, Float):
  case makeHash(UInt32, Float):
  case makeHash(Int32, Float):
  case makeHash(Double, Float):
    return "float({})";
  case makeHash(BVec2, Float):
  case makeHash(UVec2, Float):
  case makeHash(IVec2, Float):
  case makeHash(DVec2, Float):
  case makeHash(BVec3, Float):
  case makeHash(UVec3, Float):
  case makeHash(IVec3, Float):
  case makeHash(DVec3, Float):
  case makeHash(BVec4, Float):
  case makeHash(UVec4, Float):
  case makeHash(IVec4, Float):
  case makeHash(DVec4, Float):
    return "float({}.r)";
  case makeHash(Vec2, Float):
  case makeHash(Vec3, Float):
  case makeHash(Vec4, Float):
    return "{}.r";
#pragma endregion->float
#pragma region->double
  case makeHash(Bool, Double):
  case makeHash(UInt32, Double):
  case makeHash(Int32, Double):
  case makeHash(Float, Double):
    return "double({})";
  case makeHash(BVec2, Double):
  case makeHash(UVec2, Double):
  case makeHash(IVec2, Double):
  case makeHash(Vec2, Double):
  case makeHash(BVec3, Double):
  case makeHash(UVec3, Double):
  case makeHash(IVec3, Double):
  case makeHash(Vec3, Double):
  case makeHash(BVec4, Double):
  case makeHash(UVec4, Double):
  case makeHash(IVec4, Double):
  case makeHash(Vec4, Double):
    return "double({}.r)";
  case makeHash(DVec2, Double):
  case makeHash(DVec3, Double):
  case makeHash(DVec4, Double):
    return "{}.r";
#pragma endregion->double
#pragma endregion BasicTypes
#pragma region Vectors
#pragma region->bvec *
#pragma region->bvec2
  case makeHash(Bool, BVec2):
  case makeHash(UInt32, BVec2):
  case makeHash(Int32, BVec2):
  case makeHash(Float, BVec2):
  case makeHash(Double, BVec2):
  case makeHash(UVec2, BVec2):
  case makeHash(IVec2, BVec2):
  case makeHash(Vec2, BVec2):
  case makeHash(DVec2, BVec2):
    return "bvec2({})";

  case makeHash(UVec3, BVec2):
  case makeHash(IVec3, BVec2):
  case makeHash(Vec3, BVec2):
  case makeHash(DVec3, BVec2):
  case makeHash(UVec4, BVec2):
  case makeHash(IVec4, BVec2):
  case makeHash(Vec4, BVec2):
  case makeHash(DVec4, BVec2):
    return "bvec2({}.rg)";
  case makeHash(BVec3, BVec2):
  case makeHash(BVec4, BVec2):
    return "{}.rg";
#pragma endregion->bvec2
#pragma region->bvec3
  case makeHash(Bool, BVec3):
  case makeHash(UInt32, BVec3):
  case makeHash(Int32, BVec3):
  case makeHash(Float, BVec3):
  case makeHash(Double, BVec3):
  case makeHash(UVec3, BVec3):
  case makeHash(IVec3, BVec3):
  case makeHash(Vec3, BVec3):
  case makeHash(DVec3, BVec3):
    return "bvec3({})";

  case makeHash(BVec2, BVec3):
  case makeHash(UVec2, BVec3):
  case makeHash(IVec2, BVec3):
  case makeHash(Vec2, BVec3):
  case makeHash(DVec2, BVec3):
    return "bvec3({}, 0)";

  case makeHash(UVec4, BVec3):
  case makeHash(IVec4, BVec3):
  case makeHash(Vec4, BVec3):
  case makeHash(DVec4, BVec3):
    return "bvec3({}.rgb)";
  case makeHash(BVec4, BVec3):
    return "{}.rgb";
#pragma endregion->bvec3
#pragma region->bvec4
  case makeHash(Bool, BVec4):
  case makeHash(UInt32, BVec4):
  case makeHash(Int32, BVec4):
  case makeHash(Float, BVec4):
  case makeHash(Double, BVec4):
  case makeHash(UVec4, BVec4):
  case makeHash(IVec4, BVec4):
  case makeHash(Vec4, BVec4):
  case makeHash(DVec4, BVec4):
    return "bvec4({})";

  case makeHash(BVec2, BVec4):
  case makeHash(UVec2, BVec4):
  case makeHash(IVec2, BVec4):
  case makeHash(Vec2, BVec4):
  case makeHash(DVec2, BVec4):
    return "bvec4({}, 0, 0)";

  case makeHash(BVec3, BVec4):
  case makeHash(UVec3, BVec4):
  case makeHash(IVec3, BVec4):
  case makeHash(Vec3, BVec4):
  case makeHash(DVec3, BVec4):
    return "bvec4({}, 0)";
#pragma endregion->bvec4
#pragma endregion->bvec *
#pragma region->uvec *
#pragma region->uvec2
  case makeHash(Bool, UVec2):
  case makeHash(UInt32, UVec2):
  case makeHash(Int32, UVec2):
  case makeHash(Float, UVec2):
  case makeHash(Double, UVec2):
  case makeHash(BVec2, UVec2):
  case makeHash(IVec2, UVec2):
  case makeHash(Vec2, UVec2):
  case makeHash(DVec2, UVec2):
    return "uvec2({})";

  case makeHash(BVec3, UVec2):
  case makeHash(IVec3, UVec2):
  case makeHash(Vec3, UVec2):
  case makeHash(DVec3, UVec2):
  case makeHash(BVec4, UVec2):
  case makeHash(IVec4, UVec2):
  case makeHash(Vec4, UVec2):
  case makeHash(DVec4, UVec2):
    return "uvec2({}.rg)";
  case makeHash(UVec3, UVec2):
  case makeHash(UVec4, UVec2):
    return "{}.rg";
#pragma endregion->uvec2
#pragma region->uvec3
  case makeHash(Bool, UVec3):
  case makeHash(UInt32, UVec3):
  case makeHash(Int32, UVec3):
  case makeHash(Float, UVec3):
  case makeHash(Double, UVec3):
  case makeHash(BVec3, UVec3):
  case makeHash(IVec3, UVec3):
  case makeHash(Vec3, UVec3):
  case makeHash(DVec3, UVec3):
    return "uvec3({})";

  case makeHash(BVec2, UVec3):
  case makeHash(UVec2, UVec3):
  case makeHash(IVec2, UVec3):
  case makeHash(Vec2, UVec3):
  case makeHash(DVec2, UVec3):
    return "uvec3({}, 0)";

  case makeHash(BVec4, UVec3):
  case makeHash(IVec4, UVec3):
  case makeHash(Vec4, UVec3):
  case makeHash(DVec4, UVec3):
    return "uvec3({}.rgb)";
  case makeHash(UVec4, UVec3):
    return "{}.rgb";
#pragma endregion->uvec3
#pragma region->uvec4
  case makeHash(Bool, UVec4):
  case makeHash(UInt32, UVec4):
  case makeHash(Int32, UVec4):
  case makeHash(Float, UVec4):
  case makeHash(Double, UVec4):
  case makeHash(BVec4, UVec4):
  case makeHash(Vec4, UVec4):
  case makeHash(IVec4, UVec4):
  case makeHash(DVec4, UVec4):
    return "uvec4({})";

  case makeHash(BVec2, UVec4):
  case makeHash(UVec2, UVec4):
  case makeHash(IVec2, UVec4):
  case makeHash(Vec2, UVec4):
  case makeHash(DVec2, UVec4):
    return "uvec4({}, 0, 0)";

  case makeHash(BVec3, UVec4):
  case makeHash(UVec3, UVec4):
  case makeHash(IVec3, UVec4):
  case makeHash(Vec3, UVec4):
  case makeHash(DVec3, UVec4):
    return "uvec4({}, 0)";
#pragma endregion->uvec4
#pragma endregion->uvec *
#pragma region->ivec *
#pragma region->ivec2
  case makeHash(Bool, IVec2):
  case makeHash(UInt32, IVec2):
  case makeHash(Int32, IVec2):
  case makeHash(Float, IVec2):
  case makeHash(Double, IVec2):
  case makeHash(BVec2, IVec2):
  case makeHash(UVec2, IVec2):
  case makeHash(Vec2, IVec2):
  case makeHash(DVec2, IVec2):
    return "ivec2({})";

  case makeHash(BVec3, IVec2):
  case makeHash(UVec3, IVec2):
  case makeHash(Vec3, IVec2):
  case makeHash(DVec3, IVec2):
  case makeHash(BVec4, IVec2):
  case makeHash(UVec4, IVec2):
  case makeHash(Vec4, IVec2):
  case makeHash(DVec4, IVec2):
    return "ivec2({}.rg)";
  case makeHash(IVec3, IVec2):
  case makeHash(IVec4, IVec2):
    return "{}.rg";
#pragma endregion->ivec2
#pragma region->ivec3
  case makeHash(Bool, IVec3):
  case makeHash(UInt32, IVec3):
  case makeHash(Int32, IVec3):
  case makeHash(Float, IVec3):
  case makeHash(Double, IVec3):
  case makeHash(BVec3, IVec3):
  case makeHash(UVec3, IVec3):
  case makeHash(Vec3, IVec3):
  case makeHash(DVec3, IVec3):
    return "ivec3({})";

  case makeHash(BVec2, IVec3):
  case makeHash(UVec2, IVec3):
  case makeHash(IVec2, IVec3):
  case makeHash(Vec2, IVec3):
  case makeHash(DVec2, IVec3):
    return "ivec3({}, 0)";

  case makeHash(BVec4, IVec3):
  case makeHash(UVec4, IVec3):
  case makeHash(Vec4, IVec3):
  case makeHash(DVec4, IVec3):
    return "ivec3({}.rgb)";
  case makeHash(IVec4, IVec3):
    return "{}.rgb";
#pragma endregion->ivec3
#pragma region->ivec4
  case makeHash(Bool, IVec4):
  case makeHash(UInt32, IVec4):
  case makeHash(Int32, IVec4):
  case makeHash(Float, IVec4):
  case makeHash(Double, IVec4):
  case makeHash(BVec4, IVec4):
  case makeHash(UVec4, IVec4):
  case makeHash(Vec4, IVec4):
  case makeHash(DVec4, IVec4):
    return "ivec4({})";

  case makeHash(BVec2, IVec4):
  case makeHash(UVec2, IVec4):
  case makeHash(IVec2, IVec4):
  case makeHash(Vec2, IVec4):
  case makeHash(DVec2, IVec4):
    return "ivec4({}, 0, 0)";

  case makeHash(BVec3, IVec4):
  case makeHash(UVec3, IVec4):
  case makeHash(IVec3, IVec4):
  case makeHash(Vec3, IVec4):
  case makeHash(DVec3, IVec4):
    return "ivec4({}, 0)";
#pragma endregion->ivec4
#pragma endregion->ivec *
#pragma region->vec *
#pragma region->vec2
  case makeHash(Bool, Vec2):
  case makeHash(UInt32, Vec2):
  case makeHash(Int32, Vec2):
  case makeHash(Float, Vec2):
  case makeHash(Double, Vec2):
  case makeHash(BVec2, Vec2):
  case makeHash(UVec2, Vec2):
  case makeHash(IVec2, Vec2):
  case makeHash(DVec2, Vec2):
    return "vec2({})";

  case makeHash(BVec3, Vec2):
  case makeHash(UVec3, Vec2):
  case makeHash(IVec3, Vec2):
  case makeHash(DVec3, Vec2):
  case makeHash(BVec4, Vec2):
  case makeHash(UVec4, Vec2):
  case makeHash(IVec4, Vec2):
  case makeHash(DVec4, Vec2):
    return "vec2({}.rg)";
  case makeHash(Vec3, Vec2):
  case makeHash(Vec4, Vec2):
    return "{}.rg";
#pragma endregion->vec2
#pragma region->vec3
  case makeHash(Bool, Vec3):
  case makeHash(UInt32, Vec3):
  case makeHash(Int32, Vec3):
  case makeHash(Float, Vec3):
  case makeHash(Double, Vec3):
  case makeHash(BVec3, Vec3):
  case makeHash(UVec3, Vec3):
  case makeHash(IVec3, Vec3):
  case makeHash(DVec3, Vec3):
    return "vec3({})";

  case makeHash(BVec2, Vec3):
  case makeHash(UVec2, Vec3):
  case makeHash(IVec2, Vec3):
  case makeHash(Vec2, Vec3):
  case makeHash(DVec2, Vec3):
    return "vec3({}, 0)";

  case makeHash(BVec4, Vec3):
  case makeHash(UVec4, Vec3):
  case makeHash(IVec4, Vec3):
  case makeHash(DVec4, Vec3):
    return "vec3({}.rgb)";
  case makeHash(Vec4, Vec3):
    return "{}.rgb";
#pragma endregion->vec3
#pragma region->vec4
  case makeHash(Bool, Vec4):
  case makeHash(UInt32, Vec4):
  case makeHash(Int32, Vec4):
  case makeHash(Float, Vec4):
  case makeHash(Double, Vec4):
  case makeHash(BVec4, Vec4):
  case makeHash(UVec4, Vec4):
  case makeHash(IVec4, Vec4):
  case makeHash(DVec4, Vec4):
    return "vec4({})";

  case makeHash(BVec2, Vec4):
  case makeHash(UVec2, Vec4):
  case makeHash(IVec2, Vec4):
  case makeHash(Vec2, Vec4):
  case makeHash(DVec2, Vec4):
    return "vec4({}, 0, 0)";

  case makeHash(BVec3, Vec4):
  case makeHash(UVec3, Vec4):
  case makeHash(IVec3, Vec4):
  case makeHash(Vec3, Vec4):
  case makeHash(DVec3, Vec4):
    return "vec4({}, 0)";
#pragma endregion->vec4
#pragma endregion->vec *
#pragma region->dvec *
#pragma region->dvec2
  case makeHash(Bool, DVec2):
  case makeHash(UInt32, DVec2):
  case makeHash(Int32, DVec2):
  case makeHash(Float, DVec2):
  case makeHash(Double, DVec2):
  case makeHash(BVec2, DVec2):
  case makeHash(UVec2, DVec2):
  case makeHash(IVec2, DVec2):
  case makeHash(Vec2, DVec2):
    return "dvec2({})";

  case makeHash(BVec3, DVec2):
  case makeHash(UVec3, DVec2):
  case makeHash(IVec3, DVec2):
  case makeHash(Vec3, DVec2):
  case makeHash(BVec4, DVec2):
  case makeHash(UVec4, DVec2):
  case makeHash(IVec4, DVec2):
  case makeHash(Vec4, DVec2):
    return "dvec2({}.rg)";
  case makeHash(DVec3, DVec2):
  case makeHash(DVec4, DVec2):
    return "{}.rg";
#pragma endregion->dvec2
#pragma region->dvec3
  case makeHash(Bool, DVec3):
  case makeHash(UInt32, DVec3):
  case makeHash(Int32, DVec3):
  case makeHash(Float, DVec3):
  case makeHash(Double, DVec3):
  case makeHash(BVec3, DVec3):
  case makeHash(UVec3, DVec3):
  case makeHash(IVec3, DVec3):
  case makeHash(Vec3, DVec3):
    return "dvec3({})";

  case makeHash(BVec2, DVec3):
  case makeHash(UVec2, DVec3):
  case makeHash(IVec2, DVec3):
  case makeHash(Vec2, DVec3):
  case makeHash(DVec2, DVec3):
    return "dvec3({}, 0)";

  case makeHash(BVec4, DVec3):
  case makeHash(UVec4, DVec3):
  case makeHash(IVec4, DVec3):
  case makeHash(Vec4, DVec3):
    return "vec3({}.rgb)";
  case makeHash(DVec4, DVec3):
    return "{}.rgb";
#pragma endregion->dvec3
#pragma region->dvec4
  case makeHash(Bool, DVec4):
  case makeHash(UInt32, DVec4):
  case makeHash(Int32, DVec4):
  case makeHash(Float, DVec4):
  case makeHash(Double, DVec4):
  case makeHash(BVec4, DVec4):
  case makeHash(UVec4, DVec4):
  case makeHash(IVec4, DVec4):
  case makeHash(Vec4, DVec4):
    return "dvec4({})";

  case makeHash(BVec2, DVec4):
  case makeHash(UVec2, DVec4):
  case makeHash(IVec2, DVec4):
  case makeHash(Vec2, DVec4):
  case makeHash(DVec2, DVec4):
    return "dvec4({}, 0, 0)";

  case makeHash(BVec3, DVec4):
  case makeHash(UVec3, DVec4):
  case makeHash(IVec3, DVec4):
  case makeHash(Vec3, DVec4):
  case makeHash(DVec3, DVec4):
    return "dvec4({}, 0)";
#pragma endregion->dvec4
#pragma endregion->dvec *
#pragma endregion Vectors
  }

  return std::nullopt;
}
