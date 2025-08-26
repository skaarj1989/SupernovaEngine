#pragma once

#include "ResourceIndices.hpp"
#include <functional> // hash

namespace rhi {

struct SetBindingKey {
  DescriptorSetIndex set;
  BindingIndex binding;

  bool operator==(const SetBindingKey &rhs) const noexcept {
    return set == rhs.set && binding == rhs.binding;
  }
};

} // namespace rhi

namespace std {

template <> struct hash<rhi::SetBindingKey> {
  size_t operator()(const rhi::SetBindingKey &) const noexcept;
};

} // namespace std
