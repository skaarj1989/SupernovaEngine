#pragma once

#include "BasePipeline.hpp"
#include "ShaderType.hpp"
#include "PixelFormat.hpp"
#include "CompareOp.hpp"
#include "CullMode.hpp"
#include "PrimitiveTopology.hpp"
#include "VertexAttributes.hpp"

#include <limits> // numeric_limits<>
#include <string>
#include <unordered_map>
#include <optional>
#include <span>

namespace rhi {

// https://registry.khronos.org/vulkan/specs/1.3/html/chap26.html#VkStencilOp
enum class StencilOp {
  // Keeps the current value.
  Keep = VK_STENCIL_OP_KEEP,
  // Sets the value to 0.
  Zero = VK_STENCIL_OP_ZERO,
  // Sets the value to reference.
  Replace = VK_STENCIL_OP_REPLACE,
  // Increments the current value and clamps to the maximum representable
  // unsigned value.
  IncrementAndClamp = VK_STENCIL_OP_INCREMENT_AND_CLAMP,
  // Decrements the current value and clamps to 0.
  DecrementAndClamp = VK_STENCIL_OP_DECREMENT_AND_CLAMP,
  // Bitwise-inverts the current value.
  Invert = VK_STENCIL_OP_INVERT,
  // Increments the current value and wraps to 0 when the maximum value would
  // have been exceeded.
  IncrementAndWrap = VK_STENCIL_OP_INCREMENT_AND_WRAP,
  // Decrements the current value and wraps to the maximum possible value when
  // the value would go below 0.
  DecrementAndWrap = VK_STENCIL_OP_DECREMENT_AND_WRAP
};

struct StencilOpState {
  // Specifies the action performed on samples that fail the stencil test.
  StencilOp failOp{StencilOp::Keep};
  // Specifies the action performed on samples that pass both the depth and
  // stencil tests.
  StencilOp passOp{StencilOp::Keep};
  // Specifies the action performed on samples that pass the stencil test and
  // fail the depth test.
  StencilOp depthFailOp{StencilOp::Keep};
  // Specifies the comparison operator used in the stencil test.
  CompareOp compareOp{CompareOp::Always};
  // Selects the bits of the unsigned integer stencil values participating in
  // the stencil test.
  uint8_t compareMask{0xFF};
  // Selects the bits of the unsigned integer stencil values updated by the
  // stencil test in the stencil framebuffer attachment.
  uint8_t writeMask{0xFF};
  // Stencil reference value that is used in the unsigned stencil comparison.
  uint32_t reference{0};
};

struct DepthStencilState {
  // Controls whether depth testing is enabled.
  bool depthTest{false};
  // Controls whether depth writes are enabled when depthTest is true.
  // Depth writes are always disabled when depthTest is false.
  bool depthWrite{true};
  // Specifies the function used to compare each incoming pixel depth value with
  // the depth value present in the depth buffer. The comparison is performed
  // only if depth testing is enabled.
  CompareOp depthCompareOp{CompareOp::LessOrEqual};
  // Controls whether stencil testing is enabled.
  bool stencilTestEnable{false};
  StencilOpState front{};
  std::optional<StencilOpState> back{std::nullopt};
};

// https://registry.khronos.org/vulkan/specs/1.3/html/chap25.html#VkPolygonMode
enum class PolygonMode {
  Fill = VK_POLYGON_MODE_FILL,
  Line = VK_POLYGON_MODE_LINE,
  Point = VK_POLYGON_MODE_POINT
};

struct DepthBias {
  // Scalar factor controlling the constant depth value added to each fragment.
  float constantFactor{0.0f};
  // The maximum (or minimum) depth bias of a fragment.
  // Scalar factor applied to a fragment�s slope in depth bias calculations.
  float slopeFactor{0.0f};
};

struct RasterizerState {
  // The triangle rendering mode.
  PolygonMode polygonMode{PolygonMode::Fill};
  // Specify whether front- or back-facing facets can be culled.
  CullMode cullMode{CullMode::None};
  std::optional<DepthBias> depthBias;
  // Controls whether to clamp the fragment�s depth values as described in Depth
  // Test. Enabling depth clamp will also disable clipping primitives to the z
  // planes of the frustrum as described in Primitive Clipping.
  bool depthClampEnable{false};
  // The width of rasterized line segments.
  float lineWidth{1.0f};
};

// https://registry.khronos.org/vulkan/specs/1.3/html/chap27.html#VkBlendOp
enum class BlendOp {
  Add = VK_BLEND_OP_ADD,
  Subtract = VK_BLEND_OP_SUBTRACT,
  ReverseSubtract = VK_BLEND_OP_REVERSE_SUBTRACT,
  Min = VK_BLEND_OP_MIN,
  Max = VK_BLEND_OP_MAX
};
// https://registry.khronos.org/vulkan/specs/1.3/html/chap27.html#VkBlendFactor
enum class BlendFactor {
  Zero = VK_BLEND_FACTOR_ZERO,
  One = VK_BLEND_FACTOR_ONE,
  SrcColor = VK_BLEND_FACTOR_SRC_COLOR,
  OneMinusSrcColor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
  DstColor = VK_BLEND_FACTOR_DST_COLOR,
  OneMinusDstColor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
  SrcAlpha = VK_BLEND_FACTOR_SRC_ALPHA,
  OneMinusSrcAlpha = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
  DstAlpha = VK_BLEND_FACTOR_DST_ALPHA,
  OneMinusDstAlpha = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
  ConstantColor = VK_BLEND_FACTOR_CONSTANT_COLOR,
  OneMinusConstantColor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
  ConstantAlpha = VK_BLEND_FACTOR_CONSTANT_ALPHA,
  OneMinusConstantAlpha = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,
  SrcAlphaSaturate = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
  Src1Color = VK_BLEND_FACTOR_SRC1_COLOR,
  OneMinusSrc1Color = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
  Src1Alpha = VK_BLEND_FACTOR_SRC1_ALPHA,
  OneMinusSrc1Alpha = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA
};

// src = Incoming values (fragment shader output).
// dst = Values already present in a framebuffer.
struct BlendState {
  // Controls whether blending is enabled for the corresponding color
  // attachment. If blending is not enabled, the source fragment�s color for
  // that attachment is passed through unmodified.
  bool enabled{false};

  // Selects which blend factor is used to determine the source factors
  BlendFactor srcColor{BlendFactor::One};
  // Selects which blend factor is used to determine the destination factors
  BlendFactor dstColor{BlendFactor::Zero};
  // Selects which blend operation is used to calculate the RGB values to write
  // to the color attachment.
  BlendOp colorOp{BlendOp::Add};

  // Selects which blend factor is used to determine the source factor.
  BlendFactor srcAlpha{BlendFactor::One};
  // Selects which blend factor is used to determine the destination factor.
  BlendFactor dstAlpha{BlendFactor::Zero};
  // Selects which blend operation is used to calculate the alpha values to
  // write to the color attachment.
  BlendOp alphaOp{BlendOp::Add};
};

// Assign to VertexAttribute::offset in GraphicsPipeline::setInputAssembly
// to silence "Vertex attribute at location x not consumed by vertex shader".
constexpr auto kIgnoreVertexAttribute = std::numeric_limits<uint32_t>::max();

class GraphicsPipeline final : public BasePipeline {
  friend class RenderDevice;

public:
  GraphicsPipeline() = default;
  GraphicsPipeline(const GraphicsPipeline &) = delete;
  GraphicsPipeline(GraphicsPipeline &&) noexcept = default;

  GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
  GraphicsPipeline &operator=(GraphicsPipeline &&) noexcept = default;

  constexpr VkPipelineBindPoint getBindPoint() const override {
    return VK_PIPELINE_BIND_POINT_GRAPHICS;
  }

  class Builder {
  public:
    Builder();
    Builder(const Builder &) = delete;
    Builder(Builder &&) noexcept = delete;
    ~Builder() = default;

    Builder &operator=(const Builder &) = delete;
    Builder &operator=(Builder &&) noexcept = delete;

    // PixelFormat can contain stencil bits (might also be Undefined).
    Builder &setDepthFormat(const PixelFormat);
    // @param Can be empty
    Builder &setColorFormats(std::initializer_list<PixelFormat>);
    Builder &setColorFormats(std::span<const PixelFormat>);

    // Do not omit vertex attributes that your shader does not use
    // (in that case use kIgnoreVertexAttribute as an offset).
    Builder &setInputAssembly(const VertexAttributes &);
    Builder &setTopology(const PrimitiveTopology);

    Builder &setPipelineLayout(PipelineLayout);
    // If a shader of a given type is already specified, then its content will
    // be overwritten with the given code.
    Builder &addShader(const ShaderType, const std::string_view);

    Builder &setDepthStencil(const DepthStencilState &);
    Builder &setRasterizer(const RasterizerState &);
    Builder &setBlending(const AttachmentIndex, const BlendState &);
    Builder &setDynamicState(std::initializer_list<VkDynamicState>);

    [[nodiscard]] GraphicsPipeline build(RenderDevice &);

  private:
    VkFormat m_depthFormat{VK_FORMAT_UNDEFINED};
    VkFormat m_stencilFormat{VK_FORMAT_UNDEFINED};
    std::vector<VkFormat> m_colorAttachmentFormats;

    VkVertexInputBindingDescription m_vertexInput{};
    std::vector<VkVertexInputAttributeDescription> m_vertexInputAttributes;
    VkPrimitiveTopology m_primitiveTopology{
      VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};

    std::unordered_map<ShaderType, const std::string_view> m_shaderStages;
    PipelineLayout m_pipelineLayout;

    VkPipelineDepthStencilStateCreateInfo m_depthStencilState{};
    VkPipelineRasterizationStateCreateInfo m_rasterizerState{};
    std::vector<VkPipelineColorBlendAttachmentState> m_blendStates;
    std::vector<VkDynamicState> m_dynamicStates{
      VK_DYNAMIC_STATE_VIEWPORT,
      VK_DYNAMIC_STATE_SCISSOR,
    };
  };

private:
  GraphicsPipeline(const VkDevice, PipelineLayout &&, const VkPipeline);
};

} // namespace rhi
