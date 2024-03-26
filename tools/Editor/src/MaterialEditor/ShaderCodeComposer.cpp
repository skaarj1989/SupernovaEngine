#include "MaterialEditor/ShaderCodeComposer.hpp"
#include "VisitorHelper.hpp"
#include "StringUtility.hpp" // trim
#include <format>

namespace {

[[nodiscard]] auto toGLSL(const ValueVariant &in) {
  return std::visit(
    Overload{
      [](bool v) -> std::string { return v ? "true" : "false"; },
      [](glm::bvec2 v) { return std::format("bvec2({}, {})", v.x, v.y); },
      [](glm::bvec3 v) {
        return std::format("bvec3({}, {}, {})", v.x, v.y, v.z);
      },
      [](glm::bvec4 v) {
        return std::format("bvec4({}, {}, {}, {})", v.x, v.y, v.z, v.w);
      },

      [](int32_t v) { return std::to_string(v); },
      [](glm::ivec2 v) { return std::format("ivec2({}, {})", v.x, v.y); },
      [](const glm::ivec3 &v) {
        return std::format("ivec3({}, {}, {})", v.x, v.y, v.z);
      },
      [](const glm::ivec4 &v) {
        return std::format("ivec4({}, {}, {}, {})", v.x, v.y, v.z, v.w);
      },

      [](uint32_t v) { return std::to_string(v); },
      [](glm::uvec2 v) { return std::format("uvec2({}, {})", v.x, v.y); },
      [](const glm::uvec3 &v) {
        return std::format("uvec3({}, {}, {})", v.x, v.y, v.z);
      },
      [](const glm::uvec4 &v) {
        return std::format("uvec4({}, {}, {}, {})", v.x, v.y, v.z, v.w);
      },

      [](float v) { return std::to_string(v); },
      [](glm::vec2 v) { return std::format("vec2({}, {})", v.x, v.y); },
      [](const glm::vec3 &v) {
        return std::format("vec3({}, {}, {})", v.x, v.y, v.z);
      },
      [](const glm::vec4 &v) {
        return std::format("vec4({}, {}, {}, {})", v.x, v.y, v.z, v.w);
      },

      [](double v) { return std::to_string(v); },
      [](const glm::dvec2 &v) {
        return std::format("dvec2({}, {})", v.x, v.y);
      },
      [](const glm::dvec3 &v) {
        return std::format("dvec3({}, {}, {})", v.x, v.y, v.z);
      },
      [](const glm::dvec4 &v) {
        return std::format("dvec4({}, {}, {}, {})", v.x, v.y, v.z, v.w);
      },

      [](const glm::mat2 &m) {
        // clang-format off
        return std::format("mat2(\n"
                           "{}, {},\n"
                           "{}, {}\n"
                           ")",
                           m[0][0], m[0][1],
                           m[1][0], m[1][1]);
        // clang-format on
      },
      [](const glm::mat3 &m) {
        // clang-format off
        return std::format("mat3(\n"
                           "{}, {}, {},\n"
                           "{}, {}, {},\n"
                           "{}, {}, {}\n"
                           ")",
                           m[0][0], m[0][1], m[0][2],
                           m[1][0], m[1][1], m[1][2],
                           m[2][0], m[2][1], m[2][2]);
        // clang-format on
      },
      [](const glm::mat4 &m) {
        // clang-format off
        return std::format("mat4(\n"
                           "{}, {}, {}, {},\n"
                           "{}, {}, {}, {},\n"
                           "{}, {}, {}, {},\n"
                           "{}, {}, {}, {}\n"
                           ")",
                           m[0][0], m[0][1], m[0][2], m[0][3], 
                           m[1][0], m[1][1], m[1][2], m[1][3], 
                           m[2][0], m[2][1], m[2][2], m[2][3], 
                           m[3][0], m[3][1], m[3][2], m[3][3]);
        // clang-format on
      },
    },
    in);
}

} // namespace

//
// ShaderCodeComposer class:
//

ShaderCodeComposer &ShaderCodeComposer::clear() {
  m_variables.clear();
  m_code = {};
  return *this;
}

ShaderCodeComposer &ShaderCodeComposer::addVariable(const std::string_view name,
                                                    const ValueVariant &v) {
  return addVariable(getDataType(v), name, toGLSL(v));
}
ShaderCodeComposer &
ShaderCodeComposer::addVariable(const DataType dataType,
                                const std::string_view name,
                                const std::string_view value) {
  if (!m_variables.contains(name.data())) {
    addExpression(
      std::format("const {} {} = {};", toString(dataType), name, value));
    m_variables.emplace(name);
  }
  return *this;
}
ShaderCodeComposer &
ShaderCodeComposer::addExpression(const std::string_view expr) {
  m_code << expr << "\n";
  return *this;
}

std::string ShaderCodeComposer::getCode() const { return m_code.str(); }

std::string ShaderCodeComposer::makeIdentifier(std::string str) {
  trim(str);
  assert(!str.empty());

  if (auto it = str.begin(); std::isdigit(*it)) {
    str.erase(it);
  }
  for (auto &c : str) {
    if (!std::isalnum(c)) c = '_';
  }
  return str;
}
