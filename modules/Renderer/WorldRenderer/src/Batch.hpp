#pragma once

#include "renderer/Renderable.hpp"
#include <vector>

namespace gfx {

// Group of renderables for a single draw call.
struct Batch {
  const Mesh *mesh;       // VertexFormat and buffers (vertex/index).
  const SubMesh *subMesh; // Provides offsets for buffers.

  const Material *material;
  uint32_t materialOffset; // In bytes.
  TextureResources textures;

  struct InstancesInfo {
    // Offset to global (per renderpass) array of GPUInstances.
    uint32_t offset{0};
    uint32_t count{0}; // Number of instances to draw.
  };
  InstancesInfo instances;

  // Check if a renderable fits for a given batch.
  using Predicate = bool (*)(const Batch &, const Renderable &);
};

using Batches = std::vector<Batch>;

} // namespace gfx
