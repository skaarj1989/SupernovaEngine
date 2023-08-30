#pragma once

#include "RenderDevice.hpp"
#include "math/Hash.hpp"

namespace rhi {

template <class TargetPass, class PipelineType>
  requires std::is_base_of_v<BasePipeline, PipelineType>
class BasePass {
public:
  explicit BasePass(RenderDevice &rd) : m_renderDevice{rd} {}
  BasePass(const BasePass &) = delete;
  BasePass(BasePass &&) noexcept = default;
  ~BasePass() = default;

  BasePass &operator=(const BasePass &) noexcept = delete;
  BasePass &operator=(BasePass &&) noexcept = default;

  RenderDevice &getRenderDevice() const { return m_renderDevice; }

  [[nodiscard]] uint32_t count() const { return m_pipelines.size(); }
  void clear() { m_pipelines.clear(); }

protected:
  template <typename... Args> PipelineType *_getPipeline(Args &&...args) {
    std::size_t hash{0};
    (hashCombine(hash, args), ...);

    if (const auto it = m_pipelines.find(hash); it != m_pipelines.cend()) {
      return it->second.get();
    } else {
      auto pipeline = static_cast<TargetPass *>(this)->_createPipeline(
        std::forward<Args>(args)...);
      const auto &[inserted, _] = m_pipelines.emplace(
        hash, pipeline ? std::make_unique<PipelineType>(std::move(pipeline))
                       : nullptr);
      return inserted->second.get();
    }
  }

private:
  RenderDevice &m_renderDevice;

  // Key = Hashed args passed to _createPipeline.
  using PipelineCache =
    robin_hood::unordered_map<std::size_t, std::unique_ptr<PipelineType>>;
  PipelineCache m_pipelines;
};

} // namespace rhi
