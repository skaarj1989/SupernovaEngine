#pragma once

#include "MaterialEditor/VertexDescriptorSerializer.hpp"

struct Connection {
  VertexDescriptor from{nullptr};
  VertexDescriptor to{nullptr};

  [[nodiscard]] bool isValid() const { return from && to && (from != to); }

  template <class Archive> void serialize(Archive &archive) {
    archive(VertexDescriptorSerializer{from}, VertexDescriptorSerializer{to});
  }
};
