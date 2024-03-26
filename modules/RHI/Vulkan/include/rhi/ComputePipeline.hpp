#pragma once

#include "BasePipeline.hpp"
#include "glm/ext/vector_uint2.hpp"
#include "glm/ext/vector_uint3.hpp"

namespace rhi {

class RenderDevice;

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
  ComputePipeline(const VkDevice, PipelineLayout &&, const glm::uvec3 localSize,
                  const VkPipeline);

private:
  glm::uvec3 m_localSize{};
};

[[nodiscard]] glm::uvec2 calcNumWorkGroups(const glm::uvec2 extent,
                                           const glm::uvec2 localSize);
[[nodiscard]] inline auto calcNumWorkGroups(const glm::uvec2 extent,
                                            const uint32_t localSize) {
  return calcNumWorkGroups(extent, glm::uvec2{localSize});
}

} // namespace rhi
