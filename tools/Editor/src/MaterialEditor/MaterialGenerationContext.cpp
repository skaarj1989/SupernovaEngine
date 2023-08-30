#include "MaterialEditor/MaterialGenerationContext.hpp"
#include <format>

ShaderToken extractTop(std::stack<ShaderToken> &tokens) {
  assert(!tokens.empty());
  auto v = std::move(tokens.top());
  tokens.pop();
  return v;
}

[[nodiscard]] std::vector<ShaderToken> extractN(std::stack<ShaderToken> &tokens,
                                                std::size_t count) {
  std::vector<ShaderToken> args(count);
  extractN(tokens, count, args);
  return args;
}

[[nodiscard]] std::vector<DataType>
extractArgTypes(std::span<const ShaderToken> args) {
  std::vector<DataType> types(args.size());
  std::ranges::transform(args, types.begin(),
                         [](const auto &token) { return token.dataType; });
  return types;
}
[[nodiscard]] std::vector<std::string>
transform(std::span<const DataType> types) {
  std::vector<std::string> v(types.size());
  std::ranges::transform(types, v.begin(),
                         [](const auto type) { return toString(type); });
  return v;
}

[[nodiscard]] std::vector<std::string>
extractArgNames(std::span<const ShaderToken> args) {
  std::vector<std::string> names(args.size());
  std::ranges::transform(args, names.begin(),
                         [](const auto &token) { return token.name; });
  return names;
}

std::string nodeIdToString(int32_t id) {
  assert(id >= 0);
  return std::format("id_{}", std::to_string(id));
}

std::optional<std::string> assure(const ShaderToken &token,
                                  DataType requiredType) {
  assert(requiredType != DataType::Undefined);

  if (token.isValid()) {
    if (token.dataType == requiredType) {
      return token.name;
    } else if (const auto fmt =
                 getConversionFormat(token.dataType, requiredType);
               fmt) {
      return std::vformat(*fmt, std::make_format_args(token.name));
    }
  }
  return std::nullopt;
}
