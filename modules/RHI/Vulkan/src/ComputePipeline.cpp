#include "rhi/ComputePipeline.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/common.hpp" // ceil

namespace rhi {

ComputePipeline::ComputePipeline(const VkDevice device,
                                 PipelineLayout &&pipelineLayout,
                                 const glm::uvec3 localSize,
                                 const VkPipeline pipeline)
    : BasePipeline{device, std::move(pipelineLayout), pipeline},
      m_localSize{localSize} {}

glm::uvec3 ComputePipeline::getWorkGroupSize() const { return m_localSize; }

//
// Helper:
//

glm::uvec2 calcNumWorkGroups(const glm::uvec2 extent,
                             const glm::uvec2 localSize) {
  return glm::ceil(glm::vec2{extent} / glm::vec2{localSize});
}

} // namespace rhi
