#pragma once

#include "ScopedEnumFlags.hpp"
#include "glad/vulkan.h"

namespace rhi {

// https://registry.khronos.org/vulkan/specs/1.3/html/chap7.html#synchronization-pipeline-stages
enum class PipelineStages : VkPipelineStageFlags2 {
  None = VK_PIPELINE_STAGE_2_NONE,

  Top = VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT,
  VertexInput = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT,
  VertexShader = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT,
  GeometryShader = VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT,
  FragmentShader = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
  EarlyFragmentTest = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
  LateFragmentTest = VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
  FragmentTests = EarlyFragmentTest | LateFragmentTest,
  ColorAttachmentOutput = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
  ComputeShader = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
  Transfer = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
  Blit = VK_PIPELINE_STAGE_2_BLIT_BIT,
  Bottom = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,

  AllTransfer = VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT,
  AllGraphics = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT,
  AllCommands = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
};
template <> struct has_flags<PipelineStages> : std::true_type {};

} // namespace rhi
