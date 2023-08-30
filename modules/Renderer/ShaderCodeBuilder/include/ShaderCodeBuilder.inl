#include <format>

template <typename T>
inline ShaderCodeBuilder &ShaderCodeBuilder::addDefine(const std::string &name,
                                                       T &&v) {
  return addDefine(std::format("{} {}", name, v));
}
