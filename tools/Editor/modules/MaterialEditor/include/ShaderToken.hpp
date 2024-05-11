#pragma once

#include "DataType.hpp"

struct ShaderToken {
  std::string name;
  DataType dataType{DataType::Undefined};

  [[nodiscard]] constexpr bool isValid() const {
    return dataType != DataType::Undefined && !name.empty();
  }
};
