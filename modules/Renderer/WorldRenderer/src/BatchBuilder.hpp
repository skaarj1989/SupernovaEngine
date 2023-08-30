#pragma once

#include "Batch.hpp"
#include "GPUInstance.hpp"

namespace gfx {

[[nodiscard]] Batches buildBatches(std::vector<GPUInstance> &,
                                   std::span<const Renderable *>,
                                   const PropertyGroupOffsets &,
                                   Batch::Predicate);

//
// Helper:
//

[[nodiscard]] bool sameGeometry(const Batch &, const Renderable &);
[[nodiscard]] bool sameSubMesh(const Batch &, const Renderable &);
[[nodiscard]] bool sameVertexFormat(const Batch &, const Renderable &);

[[nodiscard]] bool sameMaterial(const Batch &, const Renderable &);
[[nodiscard]] bool sameTextures(const Batch &, const Renderable &);

} // namespace gfx
