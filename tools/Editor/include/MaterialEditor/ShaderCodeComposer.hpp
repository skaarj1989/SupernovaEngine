#pragma once

#include "ValueVariant.hpp"
#include "MaterialEditor/UserFunctionData.hpp"
#include <unordered_set>
#include <vector>
#include <sstream>

class ShaderCodeComposer {
public:
  ShaderCodeComposer &addVariable(const std::string_view name,
                                  const ValueVariant &);
  ShaderCodeComposer &addVariable(DataType, const std::string_view name,
                                  const std::string_view value);
  ShaderCodeComposer &addExpression(const std::string_view expr);

  ShaderCodeComposer &addFunction(const UserFunctionData *data);

  [[nodiscard]] std::string getCode() const;

  [[nodiscard]] static std::string makeIdentifier(std::string);

private:
  std::vector<const UserFunctionData *> m_userFunctions;
  std::unordered_set<std::string> m_variables;
  std::ostringstream m_code;
};
