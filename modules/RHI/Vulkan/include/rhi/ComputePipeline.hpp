#pragma once

#include "rhi/BasePipeline.hpp"
#include "glm/ext/vector_uint2.hpp"
#include "glm/ext/vector_uint3.hpp"

namespace rhi {

class ComputePipeline final : public BasePipeline {
  friend class RenderDevice; // Calls the private constructor.

public:
  ComputePipeline() = default;
  ComputePipeline(const ComputePipeline &) = delete;
  ComputePipeline(ComputePipeline &&) noexcept = default;

  ComputePipeline &operator=(const ComputePipeline &) = delete;
  ComputePipeline &operator=(ComputePipeline &&) noexcept = default;

  constexpr VkPipelineBindPoint getBindPoint() const override {
    return VK_PIPELINE_BIND_POINT_COMPUTE;
  }

  [[nodiscard]] glm::uvec3 getWorkGroupSize() const;

private:
  ComputePipeline(VkDevice, PipelineLayout &&, glm::uvec3 localSize,
                  VkPipeline);

private:
  glm::uvec3 m_localSize{};
};

[[nodiscard]] glm::uvec2 calcNumWorkGroups(glm::uvec2 extent,
                                           glm::uvec2 localSize);
[[nodiscard]] inline auto calcNumWorkGroups(glm::uvec2 extent,
                                            uint32_t localSize) {
  return calcNumWorkGroups(extent, glm::uvec2{localSize});
}

} // namespace rhi
