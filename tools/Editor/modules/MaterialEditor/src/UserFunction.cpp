#include "UserFunction.hpp"
#include "StringUtility.hpp" // join
#include "math/Hash.hpp"

#include <format>
#include <regex>
#include <span>

namespace {

[[nodiscard]] auto
buildParameters(std::span<const UserFunction::Data::Parameter> params) {
  std::vector<std::string> v;
  v.reserve(params.size());
  std::ranges::transform(params, std::back_inserter(v), [](const auto &p) {
    return std::format("{} {}", toString(p.dataType), p.name);
  });
  return join(v, ", ");
}

[[nodiscard]] bool isValid(const rhi::ShaderStages shaderStages) {
  return std::to_underlying(shaderStages) != 0;
}
[[nodiscard]] bool
isValid(std::span<const UserFunction::Data::Parameter> inputs) {
  return std::ranges::all_of(inputs, [](const auto &p) { return p.isValid(); });
}

[[nodiscard]] bool isValidGLSLIdentifier(const std::string_view identifier) {
  static const std::regex kRegexPattern("^[a-zA-Z_][a-zA-Z0-9_]*$");
  return std::regex_match(identifier.data(), kRegexPattern);
}

} // namespace

//
// UserFunction::Data::Parameter struct:
//

bool UserFunction::Data::Parameter::isValid() const {
  return dataType != DataType::Undefined && isValidGLSLIdentifier(name);
}

//
// UserFunction::Data struct:
//

bool UserFunction::Data::isValid() const {
  // clang-format off
  return
    output != DataType::Undefined &&
    ::isValid(shaderStages) &&
    !code.empty() &&
    isValidGLSLIdentifier(name) &&
    ::isValid(inputs);
  // clang-format on
}

bool operator==(const UserFunction::Data &lhs, const UserFunction::Data &rhs) {
  // clang-format off
  return 
    lhs.output == rhs.output &&
    lhs.shaderStages == rhs.shaderStages &&
    lhs.name == rhs.name &&
    lhs.code == rhs.code &&
    lhs.dependencies == rhs.dependencies &&
    lhs.inputs == rhs.inputs;
  // clang-format on
}
bool operator==(const UserFunction::Data::Parameter &lhs,
                const UserFunction::Data::Parameter &rhs) {
  return lhs.dataType == rhs.dataType && lhs.name == rhs.name;
}

//
// Helper:
//

std::string buildDeclaration(const UserFunction::Data &data) {
  return std::format("{} {}({})", toString(data.output), data.name,
                     buildParameters(data.inputs));
}
std::string buildDefinition(const UserFunction::Data &data) {
  return std::format("{} {{\n{}\n}}", buildDeclaration(data), data.code);
}

namespace std {

size_t hash<UserFunction::Data>::operator()(
  const UserFunction::Data &data) const noexcept {
  size_t h{0};
  for (const auto &p : data.inputs)
    hashCombine(h, p);

  // The code is mutable (don't hash it).
  hashCombine(h, data.output, data.name);
  return h;
}
size_t hash<UserFunction::Data::Parameter>::operator()(
  const UserFunction::Data::Parameter &p) const noexcept {
  size_t h{0};
  hashCombine(h, p.dataType, p.name);
  return h;
}

} // namespace std
