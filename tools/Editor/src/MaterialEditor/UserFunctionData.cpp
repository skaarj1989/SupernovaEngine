#include "MaterialEditor/UserFunctionData.hpp"
#include "math/Hash.hpp"

#include "StringUtility.hpp"

#include <span>
#include <format>

namespace {

[[nodiscard]] auto
buildParameters(std::span<const UserFunctionData::Parameter> params) {
  std::vector<std::string> v(params.size());
  std::ranges::transform(params, v.begin(), [](const auto &p) {
    return std::format("{} {}", toString(p.dataType), p.name);
  });
  return join(v, ", ");
}

} // namespace

std::string buildDeclaration(const UserFunctionData &data) {
  return std::format("{} {}({})", toString(data.output), data.name,
                     buildParameters(data.inputs));
}
std::string buildDefinition(const UserFunctionData &data) {
  return std::format("{} {{\n{}\n}}", buildDeclaration(data), data.code);
}

namespace std {

size_t hash<UserFunctionData>::operator()(
  const UserFunctionData &data) const noexcept {
  size_t h{0};
  for (const auto &p : data.inputs)
    hashCombine(h, p);

  // The code is mutable (don't hash it).
  hashCombine(h, data.output, data.name);
  return h;
}
size_t hash<UserFunctionData::Parameter>::operator()(
  const UserFunctionData::Parameter &p) const noexcept {
  size_t h{0};
  hashCombine(h, p.dataType, p.name);
  return h;
}

} // namespace std
