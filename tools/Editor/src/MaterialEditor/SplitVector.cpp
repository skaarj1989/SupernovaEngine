#include "MaterialEditor/SplitVector.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"

namespace {

[[nodiscard]] constexpr auto getNumRequiredChannels(SplitVector e) {
  switch (e) {
    using enum SplitVector;

  case R:
  case G:
    return 2;
  case B:
  case RGB:
    return 3;
  case A:
    return 4;
  }

  assert(false);
  return 0;
}

} // namespace

const char *getPostfix(SplitVector e) {
  // clang-format off
  switch (e) {
    using enum SplitVector;
  
  case X: return ".x";
  case Y: return ".y";
  case Z: return ".z";
  case W: return ".w";
  
  case XYZ: return ".xyz";
  }
  // clang-format on

  assert(false);
  return "";
}

NodeResult evaluate(MaterialGenerationContext &context, int32_t id,
                    SplitVector e) {
  const auto token = extractTop(context.currentShader->tokens);

  if (countChannels(token.dataType) >= getNumRequiredChannels(e)) {
    const auto baseType = getBaseDataType(token.dataType);
    const auto toSingleChannel = e != SplitVector::XYZ;

    return ShaderToken{
      .name = token.name + getPostfix(e),
      .dataType = toSingleChannel ? baseType : constructVectorType(baseType, 3),
    };
  }
  return std::unexpected{"Not enough channels."};
}
