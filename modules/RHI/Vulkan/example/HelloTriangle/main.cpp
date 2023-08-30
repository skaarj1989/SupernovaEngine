#include "os/Window.hpp"
#include "rhi/RenderDevice.hpp"
#include "glm/ext/vector_float3.hpp"
#include "spdlog/spdlog.h"

int main(int argc, char *argv[]) {
#ifdef _DEBUG
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
  _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
  _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif

  try {
    rhi::RenderDevice renderDevice;

    auto window =
      os::Window::Builder{}
        .setCaption(std::format("HelloTriangle ({})", renderDevice.getName()))
        .setPosition({0, 0})
        .setSize({640, 480})
        .build();
    os::center(window);

    rhi::VertexBuffer vertexBuffer;
    auto imageAcquired = renderDevice.createSemaphore();
    auto renderingCompleted = renderDevice.createSemaphore();

    auto swapchain = renderDevice.createSwapchain(
      window, rhi::Swapchain::Format::sRGB, rhi::VerticalSync::Enabled);

    bool quit{false};
    window.on<os::CloseWindowEvent>([&](const auto &, auto &) {
      renderDevice.destroy(renderingCompleted).destroy(imageAcquired);

      quit = true;
    });
    window.on<os::ResizeWindowEvent>(
      [&swapchain](auto, auto &) { swapchain.recreate(); });

    window.on<os::KeyboardEvent>(
      [](const os::KeyboardEvent &evt, auto &sender) {
        if (evt.keyCode == os::KeyCode::Esc && evt.state == os::KeyState::Down)
          sender.close();
      });

    struct SimpleVertex {
      glm::vec3 position;
      glm::vec3 color;
    };
    // Triangle in NDC for simplicity.
    const auto kTriangle = std::array{
      // clang-format off
      //                    position                 color 
      SimpleVertex{ {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } }, // top
      SimpleVertex{ { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f } }, // left
      SimpleVertex{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } }  // right
      // clang-format on
    };

    vertexBuffer = renderDevice.createVertexBuffer(sizeof(SimpleVertex), 3);
    {
      const auto kVerticesSize = sizeof(SimpleVertex) * kTriangle.size();
      auto stagingVertexBuffer =
        renderDevice.createStagingBuffer(kVerticesSize, kTriangle.data());

      renderDevice.execute([&](auto &cb) {
        cb.copyBuffer(stagingVertexBuffer, vertexBuffer,
                      {.size = kVerticesSize});
      });
    }

    const auto vertCode = R"(
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Color;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out vec3 v_FragColor;

void main() {
  v_FragColor = a_Color;
  gl_Position = vec4(a_Position, 1.0);
  gl_Position.y *= -1.0;
})";
    const auto fragCode = R"(
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 v_FragColor;
layout(location = 0) out vec4 FragColor;

void main() {
  FragColor = vec4(v_FragColor, 1.0);
})";

    auto graphicsPipeline =
      rhi::GraphicsPipeline::Builder{}
        .setColorFormats({swapchain.getPixelFormat()})
        .setInputAssembly({
          {0, {.type = rhi::VertexAttribute::Type::Float3, .offset = 0}},
          {1,
           {
             .type = rhi::VertexAttribute::Type::Float3,
             .offset = offsetof(SimpleVertex, color),
           }},
        })
        .addShader(rhi::ShaderType::Vertex, vertCode)
        .addShader(rhi::ShaderType::Fragment, fragCode)

        .setDepthStencil({
          .depthTest = false,
          .depthWrite = false,
        })
        .setRasterizer({.polygonMode = rhi::PolygonMode::Fill})
        .setBlending(0, {.enabled = false})
        .build(renderDevice);

    window.show();
    while (true) {
      pollEvents(&window);
      if (quit) break;

      if (!swapchain) continue;

      // The most simple (but inefficient) example: one image in flight.
      // 1. Acquire a swapchain image.
      // 2. Record to a single command buffer.
      // 3. Submit the command buffer for execution.
      // 4. Wait till the GPU finishes the job.

      swapchain.acquireNextImage(imageAcquired);
      auto &backbuffer = swapchain.getCurrentBuffer();

      auto cb = renderDevice.createCommandBuffer();
      cb.begin();

      rhi::prepareForAttachment(cb, backbuffer, false);
      const rhi::FramebufferInfo framebufferInfo{
        .area = rhi::Rect2D{.extent = backbuffer.getExtent()},
        .colorAttachments = {
          {
            .target = &backbuffer,
            .clearValue = glm::vec4{0.0f, 0.0f, 0.0f, 1.0f},
          },
        }};
      cb.beginRendering(framebufferInfo)
        .bindPipeline(graphicsPipeline)
        .draw({
          .vertexBuffer = &vertexBuffer,
          .numVertices = kTriangle.size(),
        })
        .endRendering()
        .getBarrierBuilder()
        .imageBarrier(
          {
            .image = swapchain.getCurrentBuffer(),
            .newLayout = rhi::ImageLayout::Present,
            .subresourceRange = {.levelCount = 1, .layerCount = 1},
          },
          {
            .stageMask = rhi::PipelineStages::Bottom,
            .accessMask = rhi::Access::None,
          });

      renderDevice
        .execute(cb,
                 {
                   .wait = imageAcquired,
                   .signal = renderingCompleted,
                 })
        .present(swapchain, renderingCompleted);
    }
  } catch (const std::exception &e) {
    SPDLOG_CRITICAL(e.what());
    return -1;
  }

  return 0;
}
