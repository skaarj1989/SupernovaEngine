#include "BaseApp.hpp"
#include "rhi/GraphicsPipeline.hpp"
#include "KTXLoader.hpp"
#include "glm/trigonometric.hpp"        // radians
#include "glm/gtc/matrix_transform.hpp" // perspective, translate, rotate, scale

namespace {

struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 uv;
};
constexpr auto kNumCubeVertices = 36;

} // namespace

//
// DemoApp class:
//

class DemoApp final : public BaseApp {
public:
  explicit DemoApp(std::span<char *> args)
      : BaseApp{args, {.caption = "Cube Demo"}} {
    // Use the command line argument (path to a texture file).
    // Project Properties->Configuration Properties->Debugging->CommandArguments
    if (args.size() > 1) {
      if (auto texture = loadTextureKTX(args[1], getRenderDevice()); texture) {
        m_texture = std::move(texture.value());
      } else {
        throw std::runtime_error{texture.error()};
      }
    } else {
      throw std::runtime_error{"No texture."};
    }

    m_depthStencilFormat = _findDepthStencilFormat();
    if (m_depthStencilFormat == rhi::PixelFormat::Undefined) {
      throw std::runtime_error{"Could not find suitable DepthStencil format."};
    }

    _prepareGeometry();
    _createTexturedPipeline();
    _createOutlinePipeline();
  }
  ~DemoApp() override { getRenderDevice().waitIdle(); }

private:
  void _onResizeWindow(const os::ResizeWindowEvent &evt) override {
    BaseApp::_onResizeWindow(evt);

    constexpr auto kFov = glm::radians(60.0f);
    m_projectionMatrix =
      glm::perspective(kFov, getAspectRatio(getWindow()), 0.1f, 100.0f);
    m_projectionMatrix[1][1] *= -1.0f;

    m_depthStencilBuffer = rhi::Texture::Builder{}
                             .setExtent({evt.size.x, evt.size.y})
                             .setPixelFormat(m_depthStencilFormat)
                             .setNumMipLevels(1)
                             .setUsageFlags(rhi::ImageUsage::RenderTarget)
                             .build(getRenderDevice());
  }

  void _onInput(const os::InputEvent &evt) override {
    BaseApp::_onInput(evt);
    if (const auto evt_ = std::get_if<os::KeyboardEvent>(&evt); evt_) {
      if (evt_->keyCode == os::KeyCode::Esc &&
          evt_->state == os::KeyState::Down) {
        close();
      }
    }
  }

  void _onUpdate(const fsec dt) override {
    constexpr auto kDegPerSec = 90.0f;
    m_angle += kDegPerSec * dt.count();
  }

  void _onRender(rhi::CommandBuffer &cb, const rhi::RenderTargetView rtv,
                 const fsec) override {
    constexpr auto kCubeSize = 1.2f;

    rhi::prepareForAttachment(cb, m_depthStencilBuffer, false);
    rhi::prepareForAttachment(cb, rtv.texture, false);

    // Textured cube pass:
    {
      const rhi::FramebufferInfo framebufferInfo{
        .area = {.extent = rtv.texture.getExtent()},
        .depthAttachment =
          rhi::AttachmentInfo{
            .target = &m_depthStencilBuffer,
            .clearValue = 1.0f,
          },
        .stencilAttachment =
          rhi::AttachmentInfo{
            .target = &m_depthStencilBuffer,
            .clearValue = 0u,
          },
        .colorAttachments = {{
          .target = &rtv.texture,
          .clearValue = glm::vec4{0.2f, 0.4f, 0.6f, 1.0f},
        }},
      };

      constexpr auto kDescriptorSetId = 0;

      const auto descriptorSet =
        cb.createDescriptorSetBuilder()
          .bind(0, rhi::bindings::CombinedImageSampler{&m_texture})
          .build(m_texturedPipeline.getDescriptorSetLayout(kDescriptorSetId));

      cb.bindPipeline(m_texturedPipeline)
        .bindDescriptorSet(kDescriptorSetId, descriptorSet);

      cb.beginRendering(framebufferInfo);
      _drawCube(cb, _transformCube(kCubeSize));
      cb.endRendering();
    }

    // Outline pass:
    {
      const rhi::FramebufferInfo framebufferInfo{
        .area = {.extent = rtv.texture.getExtent()},
        .depthAttachment = rhi::AttachmentInfo{.target = &m_depthStencilBuffer},
        .stencilAttachment =
          rhi::AttachmentInfo{.target = &m_depthStencilBuffer},
        .colorAttachments = {{.target = &rtv.texture}},
      };

      cb.beginRendering(framebufferInfo).bindPipeline(m_outlinePipeline);
      constexpr auto kMargin = 0.02f;
      _drawCube(cb, _transformCube(kCubeSize + kMargin));
      cb.endRendering();
    }
  }

private:
  [[nodiscard]] rhi::PixelFormat _findDepthStencilFormat() {
    using enum rhi::PixelFormat;
    for (auto format :
         {Depth32F_Stencil8, Depth24_Stencil8, Depth16_Stencil8}) {
      constexpr auto kRequiredFeatures =
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

      auto properties = getRenderDevice().getFormatProperties(format);
      if ((properties.optimalTilingFeatures & kRequiredFeatures) ==
          kRequiredFeatures) {
        return format;
      }
    }
    return Undefined;
  }

  void _prepareGeometry() {
    auto &rd = getRenderDevice();
    m_vertexBuffer = rd.createVertexBuffer(sizeof(Vertex), kNumCubeVertices);
    {
      // clang-format off
      const std::vector<Vertex> kVertices{
        // Back face
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}}, // bottom-left
        {{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}}, // top-right
        {{ 1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}}, // bottom-right
        {{ 1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}}, // top-right
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}}, // bottom-left
        {{-1.0f,  1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}}, // top-left
        // Front face
        {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},   // bottom-left
        {{ 1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},   // bottom-right
        {{ 1.0f,  1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // top-right
        {{ 1.0f,  1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // top-right
        {{-1.0f,  1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},   // top-left
        {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},   // bottom-left
        // Left face
        {{-1.0f,  1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // top-right
        {{-1.0f,  1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // top-left
        {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // bottom-left
        {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // bottom-left
        {{-1.0f, -1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // bottom-right
        {{-1.0f,  1.0f,  1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // top-right
        // Right face
        {{1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},   // top-left
        {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},   // bottom-right
        {{1.0f,  1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},   // top-right
        {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},   // bottom-right
        {{1.0f,  1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},   // top-left
        {{1.0f, -1.0f,  1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},   // bottom-left
        // Bottom face
        {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}, // top-right
        {{ 1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}}, // top-left
        {{ 1.0f, -1.0f,  1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // bottom-left
        {{ 1.0f, -1.0f,  1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // bottom-left
        {{-1.0f, -1.0f,  1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // bottom-right
        {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}, // top-right
        // Top face
        {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},   // top-left
        {{ 1.0f, 1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},   // bottom-right
        {{ 1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},   // top-right
        {{ 1.0f, 1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},   // bottom-right
        {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},   // top-left
        {{-1.0f, 1.0f,  1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}}    // bottom-left
      };
      // clang-format on

      const auto verticesSize =
        sizeof(decltype(kVertices)::value_type) * kVertices.size();
      auto stagingVertexBuffer =
        rd.createStagingBuffer(verticesSize, kVertices.data());
      rd.execute([&](auto &cb) {
        cb.copyBuffer(stagingVertexBuffer, m_vertexBuffer,
                      {.size = verticesSize});
      });
    }
  }

  void _createTexturedPipeline() {
    const auto vertCode = R"(
layout(location = 0) in vec3 a_Position;
layout(location = 2) in vec2 a_TexCoord;

layout(push_constant) uniform _PC { mat4 u_MVP; };

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out VertexData {
  vec2 texCoord;
}
vs_out;

void main() {
  vs_out.texCoord = a_TexCoord;
  gl_Position = u_MVP * vec4(a_Position, 1.0);
})";
    const auto fragCode = R"(
layout(location = 0) in VertexData {
  vec2 texCoord;
} fs_in;

layout(binding = 0) uniform sampler2D t_0;

layout(location = 0) out vec4 FragColor;
void main() {
  FragColor = texture(t_0, fs_in.texCoord);
})";

    m_texturedPipeline =
      rhi::GraphicsPipeline::Builder{}
        .setDepthFormat(m_depthStencilFormat)
        .setColorFormats({getSwapchain().getPixelFormat()})
        .setInputAssembly({
          // Position
          {0, {.type = rhi::VertexAttribute::Type::Float3, .offset = 0}},
          // Normal (not used)
          {1,
           {
             .type = rhi::VertexAttribute::Type::Float3,
             .offset = rhi::kIgnoreVertexAttribute,
           }},
          // TexCoords
          {2,
           {
             .type = rhi::VertexAttribute::Type::Float2,
             .offset = offsetof(Vertex, uv),
           }},
        })
        .addShader(rhi::ShaderType::Vertex, vertCode)
        .addShader(rhi::ShaderType::Fragment, fragCode)

        .setDepthStencil({
          .depthTest = true,
          .depthWrite = true,
          .stencilTestEnable = true,
          .front =
            {
              .failOp = rhi::StencilOp::Keep,
              .passOp = rhi::StencilOp::Replace,
              .depthFailOp = rhi::StencilOp::Keep,
              .compareOp = rhi::CompareOp::Always,
              .compareMask = 0xFF,
              .writeMask = 0xFF,
              .reference = 1,
            },
        })
        .setRasterizer({
          .polygonMode = rhi::PolygonMode::Fill,
          .cullMode = rhi::CullMode::Back,
        })
        .setBlending(0, {.enabled = false})
        .build(getRenderDevice());
  }
  void _createOutlinePipeline() {
    const auto vertCode = R"(
layout(location = 0) in vec3 a_Position;

layout(push_constant) uniform _PC { mat4 u_MVP; };

out gl_PerVertex { vec4 gl_Position; };
void main() {
  gl_Position = u_MVP * vec4(a_Position, 1.0);
})";
    const auto fragCode = R"(
layout(location = 0) out vec4 FragColor;
void main() {
  FragColor = vec4(1.0, 1.0, 1.0, 1.0);
})";

    m_outlinePipeline =
      rhi::GraphicsPipeline::Builder{}
        .setDepthFormat(m_depthStencilFormat)
        .setColorFormats({getSwapchain().getPixelFormat()})
        .setInputAssembly({
          // Position
          {0, {.type = rhi::VertexAttribute::Type::Float3, .offset = 0}},
          // Normal (not used)
          {1,
           {
             .type = rhi::VertexAttribute::Type::Float3,
             .offset = rhi::kIgnoreVertexAttribute,
           }},
          // TexCoords (not used)
          {2,
           {
             .type = rhi::VertexAttribute::Type::Float2,
             .offset = rhi::kIgnoreVertexAttribute,
           }},
        })
        .addShader(rhi::ShaderType::Vertex, vertCode)
        .addShader(rhi::ShaderType::Fragment, fragCode)

        .setDepthStencil({
          .depthTest = false,
          .depthWrite = false,
          .stencilTestEnable = true,
          .front =
            {
              .failOp = rhi::StencilOp::Keep,
              .passOp = rhi::StencilOp::Replace,
              .depthFailOp = rhi::StencilOp::Keep,
              .compareOp = rhi::CompareOp::NotEqual,
              .compareMask = 0xFF,
              .writeMask = 0x00,
              .reference = 1,
            },
        })
        .setRasterizer({
          .polygonMode = rhi::PolygonMode::Fill,
          .cullMode = rhi::CullMode::Front,
        })
        .setBlending(0, {.enabled = false})
        .build(getRenderDevice());
  }

  [[nodiscard]] glm::mat4 _transformCube(const float scale) const {
    auto modelMatrix = glm::translate(glm::mat4{1.0f}, {0.0f, 0.0f, -6.0f});
    modelMatrix =
      glm::rotate(modelMatrix, glm::radians(m_angle), {0.2f, 0.4f, 0.1f});
    modelMatrix = glm::scale(modelMatrix, glm::vec3{scale});
    return modelMatrix;
  }

  void _drawCube(rhi::CommandBuffer &cb, const glm::mat4 &modelMatrix) const {
    const auto mvp = m_projectionMatrix * modelMatrix;
    cb.pushConstants(rhi::ShaderStages::Vertex, 0, &mvp)
      .draw({.vertexBuffer = &m_vertexBuffer, .numVertices = kNumCubeVertices});
  }

private:
  rhi::VertexBuffer m_vertexBuffer;
  rhi::Texture m_texture;

  rhi::PixelFormat m_depthStencilFormat{rhi::PixelFormat::Undefined};
  rhi::Texture m_depthStencilBuffer;

  rhi::GraphicsPipeline m_texturedPipeline;
  rhi::GraphicsPipeline m_outlinePipeline;

  glm::mat4 m_projectionMatrix{1.0f};
  float m_angle{0.0f}; // In degrees.
};

CONFIG_MAIN(DemoApp);
