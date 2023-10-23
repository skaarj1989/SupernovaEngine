#include "MaterialEditor/SplitMatrix.hpp"
#include "MaterialEditor/MaterialGenerationContext.hpp"

#include <format>

namespace {

[[nodiscard]] constexpr auto getNumRequiredColumns(SplitMatrix e) {
  switch (e) {
    using enum SplitMatrix;

  case Column0:
  case Column1:
    return 2;
  case Column2:
    return 3;
  case Column3:
    return 4;
  }

  assert(false);
  return 0;
}

} // namespace

NodeResult evaluate(MaterialGenerationContext &context, int32_t id,
                    SplitMatrix e) {
  const auto token = extractTop(context.currentShader->tokens);

  if (const auto numColumns = countColumns(token.dataType);
      numColumns >= getNumRequiredColumns(e)) {
    const auto baseType = getBaseDataType(token.dataType);
    const auto columnIdx = std::to_underlying(e);

    return ShaderToken{
      .name = std::format("{}[{}]", token.name, columnIdx),
      .dataType = constructVectorType(baseType, numColumns),
    };
  }
  return std::unexpected{"Not enough columns."};
}
