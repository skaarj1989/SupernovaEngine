#pragma once

#include "ValueVariant.hpp"
#include <unordered_set>
#include <sstream>

class ShaderCodeComposer {
public:
  ShaderCodeComposer &clear();

  ShaderCodeComposer &addVariable(const std::string_view name,
                                  const ValueVariant &);
  ShaderCodeComposer &addVariable(DataType, const std::string_view name,
                                  const std::string_view value);
  ShaderCodeComposer &addExpression(const std::string_view expr);

  [[nodiscard]] std::string getCode() const;

  [[nodiscard]] static std::string makeIdentifier(std::string);

private:
  std::unordered_set<std::string> m_variables;
  std::ostringstream m_code;
};
