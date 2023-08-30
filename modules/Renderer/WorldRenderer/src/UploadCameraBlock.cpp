#include "UploadCameraBlock.hpp"
#include "UploadStruct.hpp"

namespace gfx {

namespace {

// shaders/resources/CameraBlock.glsl
struct alignas(16) GPUCameraBlock {
  GPUCameraBlock(rhi::Extent2D extent, const RawCamera &camera,
                 const ClippingPlanes &clippingPlanes)
      : projection{camera.projection},
        inversedProjection{glm::inverse(projection)}, view{camera.view},
        inversedView{glm::inverse(view)}, viewProjection{camera.viewProjection},
        inversedViewProjection{glm::inverse(viewProjection)},
        resolution{extent}, zNear{clippingPlanes.zNear},
        zFar{clippingPlanes.zFar} {}

  glm::mat4 projection{1.0f};
  glm::mat4 inversedProjection{1.0f};
  glm::mat4 view{1.0f};
  glm::mat4 inversedView{1.0f};
  glm::mat4 viewProjection{1.0f};
  glm::mat4 inversedViewProjection{1.0f};

  rhi::Extent2D resolution;

  float zNear;
  float zFar;
};
static_assert(sizeof(GPUCameraBlock) == 400);

[[nodiscard]] auto uploadCameraBlock(FrameGraph &fg,
                                     GPUCameraBlock &&cameraBlock) {
  return uploadStruct(fg, "UploadCameraBlock",
                      TransientBuffer{
                        .name = "CameraBlock",
                        .type = BufferType::UniformBuffer,
                        .data = std::move(cameraBlock),
                      });
}

} // namespace

FrameGraphResource uploadCameraBlock(FrameGraph &fg, rhi::Extent2D resolution,
                                     const PerspectiveCamera &camera) {
  return uploadCameraBlock(fg, GPUCameraBlock{
                                 resolution,
                                 RawCamera{
                                   .view = camera.getView(),
                                   .projection = camera.getProjection(),
                                   .viewProjection = camera.getViewProjection(),
                                 },
                                 camera.getClippingPlanes(),
                               });
}

FrameGraphResource uploadCameraBlock(FrameGraph &fg, rhi::Extent2D resolution,
                                     const RawCamera &camera) {
  return uploadCameraBlock(fg, GPUCameraBlock{
                                 resolution,
                                 camera,
                                 decomposeProjection(camera.projection),
                               });
}

} // namespace gfx
