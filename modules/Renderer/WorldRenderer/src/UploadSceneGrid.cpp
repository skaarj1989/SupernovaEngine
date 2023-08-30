#include "UploadSceneGrid.hpp"
#include "UploadStruct.hpp"

namespace gfx {

namespace {

// shaders/resources/SceneGridBlock.glsl
struct alignas(16) GPUSceneGrid {
  explicit GPUSceneGrid(const Grid &grid)
      : minCorner{grid.aabb.min}, gridSize{grid.size}, cellSize{grid.cellSize} {
  }

  glm::vec3 minCorner;
  float _pad{0.0f};
  glm::uvec3 gridSize;
  float cellSize;
};
static_assert(sizeof(GPUSceneGrid) == 32);

} // namespace

FrameGraphResource uploadSceneGrid(FrameGraph &fg, const Grid &grid) {
  return uploadStruct(fg, "UploadSceneGrid",
                      TransientBuffer{
                        .name = "SceneGridBlock",
                        .type = BufferType::UniformBuffer,
                        .data = GPUSceneGrid{grid},
                      });
}

} // namespace gfx
