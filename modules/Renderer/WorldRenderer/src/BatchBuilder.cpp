#include "BatchBuilder.hpp"
#include "tracy/Tracy.hpp"

namespace gfx {

Batches buildBatches(std::vector<GPUInstance> &gpuInstances,
                     std::span<const Renderable *> renderables,
                     const PropertyGroupOffsets &propertyGroupOffsets,
                     Batch::Predicate predicate) {
  ZoneScoped;

  Batches batches;
  auto lastInstanceId = gpuInstances.size();

  Batch *currentBatch{nullptr};
  for (const auto *renderable : renderables) {
    const auto &subMeshInstance = renderable->subMeshInstance;
    const auto &materialInstance = subMeshInstance.material;

    if (!currentBatch || !predicate(*currentBatch, *renderable)) {
      const auto &materialPrototype = materialInstance.getPrototype();
      const auto groupIt = propertyGroupOffsets.find(
        materialPrototype->getPropertyLayout().stride);

      currentBatch = &batches.emplace_back(Batch{
        .mesh = renderable->mesh,
        .subMesh = subMeshInstance.prototype,
        .material = materialPrototype.get(),
        .materialOffset = groupIt != propertyGroupOffsets.cend()
                            ? uint32_t(groupIt->second)
                            : 0u,
        .textures = materialInstance.getTextures(),
      });

      if (batches.size() == 1u) {
        currentBatch->instances.offset = uint32_t(lastInstanceId);
      } else {
        auto &prevBatch = batches[batches.size() - 2];
        prevBatch.instances.count =
          uint32_t(gpuInstances.size() - lastInstanceId);
        lastInstanceId += prevBatch.instances.count;
        currentBatch->instances.offset =
          prevBatch.instances.offset + prevBatch.instances.count;
      }
    }

    gpuInstances.emplace_back(GPUInstance{
      .transformId = renderable->transformId,
      .skinOffset = renderable->skinOffset,
      .materialId = renderable->materialId,
      .flags = uint32_t(materialInstance.getFlags()),
    });
  }

  if (!batches.empty()) {
    batches.back().instances.count =
      uint32_t(gpuInstances.size() - lastInstanceId);
  }
  return batches;
}

//
// Helper:
//

bool sameGeometry(const Batch &b, const Renderable &r) {
  return sameSubMesh(b, r) && sameVertexFormat(b, r);
}
bool sameSubMesh(const Batch &b, const Renderable &r) {
  return b.subMesh == r.subMeshInstance.prototype;
}
bool sameVertexFormat(const Batch &b, const Renderable &r) {
  return b.mesh->getVertexFormat() == r.mesh->getVertexFormat();
}

bool sameMaterial(const Batch &b, const Renderable &r) {
  return b.material->getHash() == r.subMeshInstance.material->getHash();
}
bool sameTextures(const Batch &b, const Renderable &r) {
  return b.textures == r.subMeshInstance.material.getTextures();
}

} // namespace gfx
