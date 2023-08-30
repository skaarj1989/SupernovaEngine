#pragma once

#include "ShaderGraphCommon.hpp"
#include "SerializationUserDataAdapter.hpp" // getUserData

// Cereal does not support raw pointer serialization.
struct VertexDescriptorSerializer {
  VertexDescriptor &vd;

  template <class Archive> int32_t save_minimal(Archive &archive) const {
    const auto &map = getUserData<const VertexDescriptorToIndexMap>(archive);
    return vd != nullptr ? map.at(vd) : kInvalidId;
  }
  template <class Archive>
  void load_minimal(Archive &archive, int32_t const &value) {
    const auto &map = getUserData<const VertexIndexToDescriptorMap>(archive);
    vd = value != kInvalidId ? map.at(value) : nullptr;
  }
};

template <class Archive>
void save(Archive &archive, std::optional<VertexDescriptor> const &in) {
  if (in) {
    auto vd = *in;
    archive(VertexDescriptorSerializer{vd});
  } else {
    archive(kInvalidId);
  }
}
template <class Archive>
void load(Archive &archive, std::optional<VertexDescriptor> &out) {
  VertexDescriptor vd{nullptr};
  archive(VertexDescriptorSerializer{vd});
  if (vd != nullptr) out = vd;
}
