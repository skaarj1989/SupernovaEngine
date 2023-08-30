#include "UploadInstances.hpp"
#include "UploadContainer.hpp"

namespace gfx {

FrameGraphResource uploadInstances(FrameGraph &fg,
                                   std::vector<GPUInstance> &&gpuInstances) {
  return uploadContainer(fg, "UploadInstances",
                         TransientBuffer{
                           .name = "Instances",
                           .type = BufferType::StorageBuffer,
                           .data = std::move(gpuInstances),
                         });
}

} // namespace gfx
