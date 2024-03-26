#include "rhi/GraphicsPipeline.hpp"
#include "rhi/RenderDevice.hpp"
#include "ShaderReflection.hpp"
#include "VkCheck.hpp"

#include "glm/common.hpp" // clamp

namespace rhi {

namespace {

[[nodiscard]] constexpr VkShaderStageFlagBits
toVk(const ShaderType shaderType) {
  switch (shaderType) {
    using enum ShaderType;

  case Vertex:
    return VK_SHADER_STAGE_VERTEX_BIT;
  case Geometry:
    return VK_SHADER_STAGE_GEOMETRY_BIT;
  case Fragment:
    return VK_SHADER_STAGE_FRAGMENT_BIT;
  case Compute:
    return VK_SHADER_STAGE_COMPUTE_BIT;
  }
  assert(false);
  return VkShaderStageFlagBits(0);
}

const VkVertexInputBindingDescription kIgnoreVertexInput{
  .binding = 0,
  .stride = 0,
  .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
};

const VkViewport kNoViewport{
  .x = 0.0f,
  .y = 0.0f,
  .width = 1.0f,
  .height = 1.0f,
  .minDepth = 0.0f,
  .maxDepth = 1.0f,
};
const VkRect2D kNoScissor{
  .offset = {.x = 0, .y = 0},
  .extent = {.width = 1, .height = 1},
};

const VkPipelineViewportStateCreateInfo kIgnoreViewportState{
  .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
  .viewportCount = 1,
  .pViewports = &kNoViewport,
  .scissorCount = 1,
  .pScissors = &kNoScissor,
};
const VkPipelineMultisampleStateCreateInfo kIgnoreMultisampleState{
  .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
  .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
  .sampleShadingEnable = VK_FALSE,
};

[[nodiscard]] auto toVk(const StencilOpState &desc) {
  return VkStencilOpState{
    .failOp = VkStencilOp(desc.failOp),
    .passOp = VkStencilOp(desc.passOp),
    .depthFailOp = VkStencilOp(desc.depthFailOp),
    .compareOp = VkCompareOp(desc.compareOp),
    .compareMask = desc.compareMask,
    .writeMask = desc.writeMask,
    .reference = desc.reference,
  };
}

[[nodiscard]] auto convert(const auto &container) {
  std::vector<VkFormat> out(container.size());
  std::ranges::transform(container, out.begin(), [](const PixelFormat format) {
    assert(getAspectMask(format) & VK_IMAGE_ASPECT_COLOR_BIT);
    return VkFormat(format);
  });
  return out;
}

} // namespace

//
// GraphicsPipeline class:
//

GraphicsPipeline::GraphicsPipeline(const VkDevice device,
                                   PipelineLayout &&pipelineLayout,
                                   const VkPipeline pipeline)
    : BasePipeline{device, std::move(pipelineLayout), pipeline} {}

//
// Builder class:
//

using Builder = GraphicsPipeline::Builder;

Builder::Builder() {
  constexpr auto kMaxNumStages = 3; // CS or VS/GS/FS
  m_shaderStages.reserve(kMaxNumStages);

  m_depthStencilState = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .depthTestEnable = VK_FALSE,
    .depthWriteEnable = VK_TRUE,
    .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
    .stencilTestEnable = VK_FALSE,
    .minDepthBounds = 0.0f,
    .maxDepthBounds = 1.0f,
  };
  m_rasterizerState = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .depthClampEnable = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_NONE,
    .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    .lineWidth = 1.0f,
  };
}

Builder &Builder::setDepthFormat(const PixelFormat depthFormat) {
  const auto aspectMask = getAspectMask(depthFormat);
  m_depthFormat = aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT ? VkFormat(depthFormat)
                                                         : VK_FORMAT_UNDEFINED;
  m_stencilFormat = aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT
                      ? VkFormat(depthFormat)
                      : VK_FORMAT_UNDEFINED;
  return *this;
}

Builder &Builder::setColorFormats(std::initializer_list<PixelFormat> formats) {
  m_colorAttachmentFormats = convert(formats);
  return *this;
}
Builder &Builder::setColorFormats(std::span<const PixelFormat> formats) {
  m_colorAttachmentFormats = convert(formats);
  return *this;
}

Builder &Builder::setInputAssembly(const VertexAttributes &vertexAttributes) {
  m_vertexInputAttributes.clear();

  if (!vertexAttributes.empty()) {
    m_vertexInputAttributes.reserve(vertexAttributes.size());

    uint32_t stride{0};
    for (const auto &[location, attrib] : vertexAttributes) {
      if (attrib.offset != kIgnoreVertexAttribute) {
        m_vertexInputAttributes.push_back({
          .location = location,
          .binding = 0,
          .format = VkFormat(attrib.type),
          .offset = attrib.offset,
        });
      }
      stride += getSize(attrib.type);
    }
    m_vertexInput = {
      .binding = 0,
      .stride = stride,
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };
  } else {
    m_vertexInput = kIgnoreVertexInput;
  }
  return *this;
}
Builder &Builder::setTopology(const PrimitiveTopology topology) {
  m_primitiveTopology = VkPrimitiveTopology(topology);
  return *this;
}

Builder &Builder::setPipelineLayout(PipelineLayout pipelineLayout) {
  m_pipelineLayout = std::move(pipelineLayout);
  return *this;
}
Builder &Builder::addShader(const ShaderType type,
                            const std::string_view code) {
  // Again, Sonarlint is wrong, DO NOT replace 'emplace' with 'try_emplace'.
  // A builder might be used to create multiple pipelines, hence it's
  // necessary to REPLACE a given shader with another one (try_emplace prevents
  // that).
  m_shaderStages.emplace(type, code);
  return *this;
}

Builder &Builder::setDepthStencil(const DepthStencilState &desc) {
  m_depthStencilState.depthTestEnable = desc.depthTest;
  m_depthStencilState.depthWriteEnable = desc.depthWrite;
  m_depthStencilState.depthCompareOp = VkCompareOp(desc.depthCompareOp);

  m_depthStencilState.stencilTestEnable = desc.stencilTestEnable;
  m_depthStencilState.front = toVk(desc.front);
  m_depthStencilState.back = toVk(desc.back.value_or(desc.front));
  return *this;
}
Builder &Builder::setRasterizer(const RasterizerState &desc) {
  m_rasterizerState.depthClampEnable = desc.depthClampEnable;
  m_rasterizerState.polygonMode = VkPolygonMode(desc.polygonMode);
  m_rasterizerState.cullMode = VkCullModeFlags(desc.cullMode);
  if (desc.depthBias) {
    m_rasterizerState.depthBiasEnable = VK_TRUE;
    m_rasterizerState.depthBiasConstantFactor = desc.depthBias->constantFactor;
    m_rasterizerState.depthBiasSlopeFactor = desc.depthBias->slopeFactor;
  }
  m_rasterizerState.lineWidth = desc.lineWidth;
  return *this;
}
Builder &Builder::setBlending(const AttachmentIndex index,
                              const BlendState &desc) {
  if (index >= m_blendStates.size()) m_blendStates.resize(index + 1);

  m_blendStates[index] = VkPipelineColorBlendAttachmentState{
    .blendEnable = desc.enabled,
    .srcColorBlendFactor = VkBlendFactor(desc.srcColor),
    .dstColorBlendFactor = VkBlendFactor(desc.dstColor),
    .colorBlendOp = VkBlendOp(desc.colorOp),
    .srcAlphaBlendFactor = VkBlendFactor(desc.srcAlpha),
    .dstAlphaBlendFactor = VkBlendFactor(desc.dstAlpha),
    .alphaBlendOp = VkBlendOp(desc.alphaOp),
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT};
  return *this;
}
Builder &
Builder::setDynamicState(std::initializer_list<VkDynamicState> states) {
  m_dynamicStates = states;
  return *this;
}

GraphicsPipeline Builder::build(RenderDevice &rd) {
  // -- Dynamic rendering:

  const VkPipelineRenderingCreateInfoKHR renderingInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
    .colorAttachmentCount = uint32_t(m_colorAttachmentFormats.size()),
    .pColorAttachmentFormats = m_colorAttachmentFormats.data(),
    .depthAttachmentFormat = m_depthFormat,
    .stencilAttachmentFormat = m_stencilFormat,
  };

  // -- Vertex Attributes:

  const VkPipelineVertexInputStateCreateInfo vertexInputStateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .vertexBindingDescriptionCount = m_vertexInput.stride > 0 ? 1u : 0u,
    .pVertexBindingDescriptions = &m_vertexInput,
    .vertexAttributeDescriptionCount = uint32_t(m_vertexInputAttributes.size()),
    .pVertexAttributeDescriptions = m_vertexInputAttributes.data(),
  };
  const VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .topology = m_primitiveTopology,
    .primitiveRestartEnable = VK_FALSE,
  };

  // --

  const auto lineWidthRange = rd.getDeviceLimits().lineWidthRange;
  m_rasterizerState.lineWidth = glm::clamp(
    m_rasterizerState.lineWidth, lineWidthRange[0], lineWidthRange[1]);

  // -- Shader stages:

  const auto numShaderStages = m_shaderStages.size();
  assert(numShaderStages > 0);

  auto reflection =
    m_pipelineLayout ? std::nullopt : std::make_optional<ShaderReflection>();

  std::vector<ShaderModule> shaderModules; // For delayed destruction only.
  shaderModules.reserve(numShaderStages);
  std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
  shaderStages.reserve(numShaderStages);
  for (const auto &[shaderType, code] : m_shaderStages) {
    auto shaderModule = rd.createShaderModule(
      shaderType, code,
      reflection ? std::addressof(reflection.value()) : nullptr);
    if (!shaderModule) continue;

    shaderStages.push_back(VkPipelineShaderStageCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .stage = toVk(shaderType),
      .module = VkShaderModule{shaderModule},
      .pName = "main",
    });

    shaderModules.emplace_back(std::move(shaderModule));
  }
  if (shaderStages.size() != numShaderStages) return {};

  if (reflection) m_pipelineLayout = reflectPipelineLayout(rd, *reflection);
  assert(m_pipelineLayout);

  // -- Blending state:

  assert(m_blendStates.size() == m_colorAttachmentFormats.size());

  const VkPipelineColorBlendStateCreateInfo colorBlendInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .logicOpEnable = VK_FALSE,
    .logicOp = VK_LOGIC_OP_CLEAR,
    .attachmentCount = uint32_t(m_blendStates.size()),
    .pAttachments = m_blendStates.data(),
    .blendConstants = {0.0f},
  };

  // -- Dynamic state:

  const VkPipelineDynamicStateCreateInfo dynamicStateInfo{
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = uint32_t(m_dynamicStates.size()),
    .pDynamicStates = m_dynamicStates.data(),
  };

  // -- Assemble:

  const VkGraphicsPipelineCreateInfo graphicsPipelineInfo{
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext = &renderingInfo,

    .stageCount = uint32_t(shaderStages.size()),
    .pStages = shaderStages.data(),

    .pVertexInputState = &vertexInputStateInfo,
    .pInputAssemblyState = &inputAssemblyInfo,

    .pViewportState = &kIgnoreViewportState,

    .pRasterizationState = &m_rasterizerState,
    .pMultisampleState = &kIgnoreMultisampleState,
    .pDepthStencilState = &m_depthStencilState,
    .pColorBlendState = &colorBlendInfo,
    .pDynamicState = &dynamicStateInfo,

    .layout = m_pipelineLayout.getHandle(),

    .renderPass = VK_NULL_HANDLE,

    .subpass = 0,
    .basePipelineHandle = VK_NULL_HANDLE,
  };

  const auto device = rd._getLogicalDevice();

  VkPipeline handle{VK_NULL_HANDLE};
  VK_CHECK(vkCreateGraphicsPipelines(device, rd._getPipelineCache(), 1,
                                     &graphicsPipelineInfo, nullptr, &handle));

  return GraphicsPipeline{device, std::move(m_pipelineLayout), handle};
}

} // namespace rhi
