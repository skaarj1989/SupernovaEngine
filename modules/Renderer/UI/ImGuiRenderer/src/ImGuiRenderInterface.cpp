#include "ImGuiRenderInterface.hpp"
#include "os/Window.hpp"
#include "rhi/RenderDevice.hpp"
#include "rhi/FrameController.hpp"
#include "ImGuiRenderer.hpp"
#include "imgui.h"
#include "glm/ext/vector_float3.hpp"

#ifdef IMGUI_HAS_VIEWPORT
namespace {

constexpr auto kFramesInFlight = 2;

[[nodiscard]] auto *getWindow(const ImGuiViewport *viewport) {
  return static_cast<os::Window *>(viewport->PlatformHandle);
}
[[nodiscard]] auto &getRenderer() {
  return *static_cast<ImGuiRenderer *>(ImGui::GetIO().BackendRendererUserData);
}

struct ImGuiViewportData {
  ImGuiViewportData(const ImGuiRenderer &renderer, const os::Window &window) {
    resources = renderer.createResources(kFramesInFlight);
    auto &rd = renderer.getRenderDevice();
    swapchain = rd.createSwapchain(window, rhi::Swapchain::Format::Linear,
                                   rhi::VerticalSync::Enabled);
    frameController = rhi::FrameController{rd, swapchain, kFramesInFlight};
  }

  // Command buffers (members of FrameController) must be released before
  // FrameResources (vertex and index buffers).
  std::vector<ImGuiRenderer::FrameResources> resources;
  rhi::Swapchain swapchain;
  rhi::FrameController frameController;
};

[[nodiscard]] auto &getViewportData(ImGuiViewport *viewport) {
  return *static_cast<ImGuiViewportData *>(viewport->RendererUserData);
}

} // namespace
#endif

void setupRenderInterface(ImGuiRenderer &renderer) {
  auto &io = ImGui::GetIO();
  static const auto backendRendererName = renderer.getRenderDevice().getName();
  io.BackendRendererName = backendRendererName.c_str();
  io.BackendRendererUserData = &renderer;

#ifdef IMGUI_HAS_VIEWPORT
  io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
  if (!(io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)) return;

  auto &platformIo = ImGui::GetPlatformIO();
  platformIo.Renderer_CreateWindow = [](ImGuiViewport *viewport) {
    auto parentViewport = ImGui::FindViewportByID(viewport->ParentViewportId);
    assert(parentViewport);
    viewport->RendererUserData =
      new ImGuiViewportData{getRenderer(), *getWindow(viewport)};
  };
  platformIo.Renderer_DestroyWindow = [](ImGuiViewport *viewport) {
    if (!(viewport->Flags & ImGuiViewportFlags_OwnedByApp)) {
      assert(viewport->RendererUserData != nullptr);
      delete std::bit_cast<ImGuiViewportData *>(viewport->RendererUserData);
    }
    viewport->RendererUserData = nullptr;
  };
  platformIo.Renderer_SetWindowSize = [](ImGuiViewport *viewport, ImVec2) {
#  if 0
    // Ugly blink, swapchain is recreated in:
    // - RenderDevice::present
    // - Swapchain::acquireNextImage
    getViewportData(viewport).swapchain.recreate();
#  endif
  };
  platformIo.Renderer_RenderWindow = [](ImGuiViewport *viewport, void *) {
    auto &[resources, _, frameController] = getViewportData(viewport);

    auto &cb = frameController.beginFrame();
    {
      RHI_GPU_ZONE(cb, "ImGui::External");
      auto &&[frameIndex, backbuffer] = frameController.getCurrentTarget();

      rhi::prepareForAttachment(cb, backbuffer, false);
      constexpr auto kClearColor = glm::vec3{0.0f};
      cb.beginRendering({
        .area = {.extent = backbuffer.getExtent()},
        .colorAttachments = {{
          .target = &backbuffer,
          .clearValue = viewport->Flags & ImGuiViewportFlags_NoRendererClear
                          ? std::nullopt
                          : std::make_optional(glm::vec4{kClearColor, 1.0f}),
        }},
      });
      getRenderer().draw(cb, backbuffer.getPixelFormat(), resources[frameIndex],
                         viewport->DrawData);
      cb.endRendering();
    }
    frameController.endFrame();
  };
  platformIo.Renderer_SwapBuffers = [](ImGuiViewport *viewport, void *) {
    getViewportData(viewport).frameController.present();
  };
#endif
}
