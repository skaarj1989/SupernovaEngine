#include "RmlUiRenderInterface.hpp"
#include "os/FileSystem.hpp"
#include "STBImageLoader.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace {

[[nodiscard]] glm::mat4 buildProjectionMatrix(const rhi::Extent2D extent) {
  const auto L = 0.0f;
  const auto R = float(extent.width);
  auto B = float(extent.height);
  auto T = 0.0f;
  std::swap(T, B);

  // clang-format off
  return {
    { 2.0f/(R-L),   0.0f,         0.0f,  0.0f },
    { 0.0f,         2.0f/(T-B),   0.0f,  0.0f },
    { 0.0f,         0.0f,        -1.0f,  0.0f },
    { (R+L)/(L-R),  (T+B)/(B-T),  0.0f,  1.0f },
  };
  // clang-format on
}

} // namespace

//
// RmlUiRenderData struct:
//

RmlUiRenderData::RmlUiRenderData(
  const RmlUiRenderer &renderer,
  const rhi::FrameIndex::ValueType numFramesInFlight)
    : frameResources{renderer.createResources(numFramesInFlight)},
      frameIndex{numFramesInFlight} {}
RmlUiRenderData::RmlUiRenderData(const RmlUiRenderData &) : RmlUiRenderData{} {}

void RmlUiRenderData::resize(const rhi::Extent2D &extent) {
  projection = buildProjectionMatrix(extent);
  scissorOriginal = {.offset = {0, 0}, .extent = extent};
  scissor = scissorOriginal;
}
RmlUiRenderer::FrameResources *RmlUiRenderData::swapBuffers() {
  auto *resources = &frameResources[frameIndex];
  resources->vertexOffset = 0;
  resources->indexOffset = 0;
  ++frameIndex;
  return resources;
}

void RmlUiRenderData::transform(std::optional<glm::mat4> m) {
  uniforms.transform = m ? projection * m.value() : projection;
}
void RmlUiRenderData::translate(const glm::vec2 v) { uniforms.translate = v; }

//
// RmlUiRenderInterface class:
//

RmlUiRenderInterface::RmlUiRenderInterface(rhi::RenderDevice &rd)
    : m_renderer{std::make_unique<RmlUiRenderer>(rd)} {}

void RmlUiRenderInterface::Set(rhi::CommandBuffer &cb, rhi::Texture &target,
                               RmlUiRenderData &renderData) {
  m_commandBuffer = &cb;
  m_target = &target;
  m_renderData = &renderData;

  m_renderData->resize(m_target->getExtent());
  m_frameResources = m_renderData->swapBuffers();

  SetTransform(nullptr);
}

void RmlUiRenderInterface::RenderGeometry(Rml::Vertex *vertices,
                                          int32_t numVertices, int32_t *indices,
                                          int32_t numIndices,
                                          Rml::TextureHandle textureHandle,
                                          const Rml::Vector2f &translation) {
  m_renderData->translate({translation.x, translation.y});
  m_renderer->draw(
    *m_commandBuffer, m_target->getPixelFormat(), *m_frameResources,
    {
      .vertices = std::span{vertices, std::size_t(numVertices)},
      .indices = std::span{indices, std::size_t(numIndices)},
    },
    std::bit_cast<const rhi::Texture *>(textureHandle), m_renderData->uniforms);
}

void RmlUiRenderInterface::EnableScissorRegion(bool enable) {
  m_renderData->isUseScissorSpecified = enable;
  if (m_renderData->isUseScissorSpecified == false)
    m_commandBuffer->setScissor(m_renderData->scissorOriginal);
}

void RmlUiRenderInterface::SetScissorRegion(int x, int y, int width,
                                            int height) {
  if (m_renderData->isUseScissorSpecified &&
      !m_renderData->isTransformEnabled) {
    m_renderData->scissor = {
      .offset = {.x = std::abs(x), .y = std::abs(y)},
      .extent = {.width = uint32_t(width), .height = uint32_t(height)},
    };
    m_commandBuffer->setScissor(m_renderData->scissor);
  }
}

bool RmlUiRenderInterface::LoadTexture(Rml::TextureHandle &handle,
                                       Rml::Vector2i &dimensions,
                                       const Rml::String &source) {
  if (auto texture = loadTextureSTB(os::FileSystem::getRoot() / source,
                                    m_renderer->getRenderDevice());
      texture) {
    const auto extent = texture->getExtent();
    dimensions = {int32_t(extent.width), int32_t(extent.height)};
    auto *ptr = new rhi::Texture{std::move(texture.value())};
    handle = Rml::TextureHandle(ptr);
    return true;
  }
  return false;
}
bool RmlUiRenderInterface::GenerateTexture(Rml::TextureHandle &handle,
                                           const Rml::byte *pixels,
                                           const Rml::Vector2i &dimensions) {
  auto &rd = m_renderer->getRenderDevice();

  const auto uploadSize = dimensions.x * dimensions.y * 4 * sizeof(Rml::byte);
  auto stagingBuffer = rd.createStagingBuffer(uploadSize, pixels);

  auto font =
    rhi::Texture::Builder{}
      .setExtent({
        .width = uint32_t(dimensions.x),
        .height = uint32_t(dimensions.y),
      })
      .setPixelFormat(rhi::PixelFormat::RGBA8_UNorm)
      .setNumMipLevels(1)
      .setNumLayers(std::nullopt)
      .setUsageFlags(rhi::ImageUsage::TransferDst | rhi::ImageUsage::Sampled)
      .setupOptimalSampler(false)
      .build(rd);

  rd.execute([&stagingBuffer, &font](auto &cb) {
      cb.copyBuffer(stagingBuffer, font);
      rhi::prepareForReading(cb, font);
    })
    .setupSampler(font, {
                          .magFilter = rhi::TexelFilter::Linear,
                          .minFilter = rhi::TexelFilter::Linear,
                          .mipmapMode = rhi::MipmapMode::Nearest,
                          .addressModeS = rhi::SamplerAddressMode::Repeat,
                          .addressModeT = rhi::SamplerAddressMode::Repeat,
                          .addressModeR = rhi::SamplerAddressMode::Repeat,
                          .minLod = 0.0f,
                          .maxLod = 1.0f,
                        });

  auto *ptr = new rhi::Texture{std::move(font)};
  handle = Rml::TextureHandle(ptr);

  return true;
}
void RmlUiRenderInterface::ReleaseTexture(Rml::TextureHandle handle) {
  auto *ptr = std::bit_cast<rhi::Texture *>(handle);
  delete ptr;
}

void RmlUiRenderInterface::SetTransform(const Rml::Matrix4f *m) {
  m_renderData->transform(m ? std::make_optional(glm::make_mat4(m->data()))
                            : std::nullopt);
}

RmlUiRenderer &RmlUiRenderInterface::GetRenderer() { return *m_renderer; }

RmlUiRenderData RmlUiRenderInterface::CreateRenderData(
  const rhi::FrameIndex::ValueType numFrames) const {
  return RmlUiRenderData{*m_renderer, numFrames};
}
