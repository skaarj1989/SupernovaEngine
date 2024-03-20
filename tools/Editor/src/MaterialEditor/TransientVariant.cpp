#include "MaterialEditor/TransientVariant.hpp"

DataType getDataType(const TransientVariant &v) {
  return std::visit(
    [](const auto &arg) {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_empty_v<T>) {
        return DataType::Undefined;
      } else {
        return ::getDataType(arg);
      }
    },
    v);
}
