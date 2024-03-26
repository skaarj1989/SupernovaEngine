#include "UploadTransforms.hpp"
#include "UploadContainer.hpp"

#include "fg/Blackboard.hpp"
#include "FrameGraphData/Transforms.hpp"

#include "glm/ext/matrix_float4x4.hpp"

namespace gfx {

void uploadTransforms(FrameGraph &fg, FrameGraphBlackboard &blackboard,
                      std::vector<glm::mat4> &&transforms) {
  const auto buffer = uploadContainer(fg, "UploadTransforms",
                                      TransientBuffer{
                                        .name = "Transforms",
                                        .type = BufferType::StorageBuffer,
                                        .data = std::move(transforms),
                                      });
  if (buffer) blackboard.add<TransformData>(*buffer);
}

} // namespace gfx
