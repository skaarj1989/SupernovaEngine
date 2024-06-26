#pragma once

#include "PipelineLayout.hpp"

namespace rhi {

class BasePipeline {
public:
  BasePipeline() = default;
  BasePipeline(const BasePipeline &) = delete;
  BasePipeline(BasePipeline &&) noexcept;
  virtual ~BasePipeline();

  BasePipeline &operator=(const BasePipeline &) = delete;
  BasePipeline &operator=(BasePipeline &&) noexcept;

  [[nodiscard]] explicit operator bool() const;

  [[nodiscard]] VkPipeline getHandle() const;
  [[nodiscard]] constexpr virtual VkPipelineBindPoint getBindPoint() const = 0;

  [[nodiscard]] const PipelineLayout &getLayout() const;
  [[nodiscard]] VkDescriptorSetLayout
  getDescriptorSetLayout(const DescriptorSetIndex) const;

protected:
  BasePipeline(const VkDevice, PipelineLayout &&, const VkPipeline);

private:
  void _destroy() noexcept;

private:
  VkDevice m_device{VK_NULL_HANDLE};
  PipelineLayout m_layout;
  VkPipeline m_handle{VK_NULL_HANDLE};
};

} // namespace rhi
