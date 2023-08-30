#include "UploadMaterialProperties.hpp"
#include "UploadContainer.hpp"

namespace gfx {

FrameGraphResource uploadMaterialProperties(FrameGraph &fg,
                                            std::vector<std::byte> &&blob) {
  return uploadContainer(fg, "UploadMaterialProperties",
                         TransientBuffer{
                           .name = "MaterialProperties",
                           .type = BufferType::StorageBuffer,
                           .data = std::move(blob),
                         });
}

} // namespace gfx
