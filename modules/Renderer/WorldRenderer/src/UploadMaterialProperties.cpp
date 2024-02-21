#include "UploadMaterialProperties.hpp"
#include "UploadContainer.hpp"

#include "fg/Blackboard.hpp"
#include "FrameGraphData/MaterialProperties.hpp"

namespace gfx {

void uploadMaterialProperties(FrameGraph &fg, FrameGraphBlackboard &blackboard,
                              std::vector<std::byte> &&blob) {
  if (const auto buffer = uploadMaterialProperties(fg, std::move(blob));
      buffer) {
    blackboard.add<MaterialPropertiesData>(*buffer);
  }
}
std::optional<FrameGraphResource>
uploadMaterialProperties(FrameGraph &fg, std::vector<std::byte> &&blob) {
  return uploadContainer(fg, "UploadMaterialProperties",
                         TransientBuffer{
                           .name = "MaterialProperties",
                           .type = BufferType::StorageBuffer,
                           .data = std::move(blob),
                         });
}

} // namespace gfx
