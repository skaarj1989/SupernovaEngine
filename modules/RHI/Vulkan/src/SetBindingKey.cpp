#include "rhi/SetBindingKey.hpp"

namespace std {

size_t hash<rhi::SetBindingKey>::operator()(
  const rhi::SetBindingKey &key) const noexcept {
  return (static_cast<size_t>(key.set) << 32) ^
         static_cast<size_t>(key.binding);
}

} // namespace std
