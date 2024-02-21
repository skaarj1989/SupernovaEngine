#include "UploadSkins.hpp"
#include "UploadContainer.hpp"

#include "fg/Blackboard.hpp"
#include "FrameGraphData/Skins.hpp"

#include "glm/mat4x4.hpp"

namespace gfx {

void uploadSkins(FrameGraph &fg, FrameGraphBlackboard &blackboard,
                 std::vector<glm::mat4> &&skins) {
  const auto buffer = uploadContainer(fg, "UploadSkins",
                                      TransientBuffer{
                                        .name = "Skins",
                                        .type = BufferType::StorageBuffer,
                                        .data = std::move(skins),
                                      });
  if (buffer) blackboard.add<SkinData>(*buffer);
}

} // namespace gfx
