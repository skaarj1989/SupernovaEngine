#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "Technique.hpp"
#include "MaterialInstance.hpp"

namespace gfx {

struct CommonSamplers;

class PostProcessor final : public rhi::RenderPass<PostProcessor>,
                            public Technique {
  friend class BasePass;

public:
  PostProcessor(rhi::RenderDevice &, const CommonSamplers &);

  uint32_t count(PipelineGroups) const override;
  void clear(PipelineGroups) override;

  [[nodiscard]] FrameGraphResource addPass(FrameGraph &,
                                           const FrameGraphBlackboard &,
                                           FrameGraphResource sceneColor,
                                           const MaterialInstance &);

  [[nodiscard]] static std::string buildFragmentCode(const rhi::RenderDevice &,
                                                     const Material &);

  struct PassInfo {
    rhi::PixelFormat colorFormat;
    const Material *material;
  };

private:
  [[nodiscard]] rhi::GraphicsPipeline _createPipeline(const PassInfo &) const;

private:
  const CommonSamplers &m_samplers;
};

} // namespace gfx
