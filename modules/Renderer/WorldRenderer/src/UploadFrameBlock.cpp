#include "UploadFrameBlock.hpp"
#include "UploadStruct.hpp"

#include "fg/Blackboard.hpp"
#include "FrameGraphData/Frame.hpp"

namespace gfx {

// shaders/resources/FrameBlock.glsl
struct alignas(16) GPUFrameBlock {
  explicit GPUFrameBlock(const FrameInfo &i)
      : time{i.time}, deltaTime{i.deltaTime} {}

  float time{0.0f};
  float deltaTime{0.0f};
};
static_assert(sizeof(GPUFrameBlock) == 16);

void uploadFrameBlock(FrameGraph &fg, FrameGraphBlackboard &blackboard,
                      const FrameInfo &frameInfo) {
  const auto buffer = uploadStruct(fg, "UploadFrameBlock",
                                   TransientBuffer{
                                     .name = "FrameBlock",
                                     .type = BufferType::UniformBuffer,
                                     .data = GPUFrameBlock{frameInfo},
                                   });
  blackboard.add<FrameData>(buffer);
}

} // namespace gfx
