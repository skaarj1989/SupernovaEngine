#pragma once

#include "rhi/ComputePass.hpp"
#include "Technique.hpp"

namespace gfx {

class IBL final : public Technique {
public:
  explicit IBL(rhi::RenderDevice &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  [[nodiscard]] rhi::Texture generateBRDF(rhi::CommandBuffer &);

  [[nodiscard]] decltype(auto) generateIrradiance(rhi::CommandBuffer &cb,
                                                  const rhi::Texture &source) {
    return m_irradianceGenerator.generate(cb, source);
  }
  [[nodiscard]] decltype(auto) prefilterEnvMap(rhi::CommandBuffer &cb,
                                               const rhi::Texture &source) {
    return m_prefilterGenerator.generate(cb, source);
  }

  class IrradianceGenerator final
      : public rhi::ComputePass<IrradianceGenerator> {
    friend class BasePass;

  public:
    explicit IrradianceGenerator(rhi::RenderDevice &);

    [[nodiscard]] rhi::Texture generate(rhi::CommandBuffer &,
                                        const rhi::Texture &);

  private:
    [[nodiscard]] rhi::ComputePipeline _createPipeline() const;
  };
  class PrefilterGenerator final : public rhi::ComputePass<PrefilterGenerator> {
    friend class BasePass;

  public:
    explicit PrefilterGenerator(rhi::RenderDevice &);

    [[nodiscard]] rhi::Texture generate(rhi::CommandBuffer &,
                                        const rhi::Texture &);

  private:
    [[nodiscard]] rhi::ComputePipeline
    _createPipeline(uint32_t numMipLevels) const;
  };

private:
  rhi::RenderDevice &m_renderDevice;

  rhi::ComputePipeline m_brdfPipeline;

  IrradianceGenerator m_irradianceGenerator{m_renderDevice};
  PrefilterGenerator m_prefilterGenerator{m_renderDevice};
};

} // namespace gfx
