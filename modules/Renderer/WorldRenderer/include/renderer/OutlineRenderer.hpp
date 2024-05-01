#pragma once

#include "fg/Fwd.hpp"
#include "rhi/RenderPass.hpp"
#include "rhi/Extent2D.hpp"
#include "Technique.hpp"

namespace gfx {

struct ViewInfo;
struct BaseGeometryPassInfo;

class VertexFormat;

class OutlineRenderer : public Technique {
public:
  explicit OutlineRenderer(rhi::RenderDevice &);

  uint32_t count(const PipelineGroups) const override;
  void clear(const PipelineGroups) override;

  FrameGraphResource addOutlines(FrameGraph &, FrameGraphBlackboard &,
                                 const ViewInfo &,
                                 const FrameGraphResource target);

private:
  class SilhouettePass final : public rhi::RenderPass<SilhouettePass>,
                               public Technique {
    friend class BasePass;

  public:
    explicit SilhouettePass(rhi::RenderDevice &);

    void addGeometryPass(FrameGraph &, FrameGraphBlackboard &,
                         const rhi::Extent2D resolution, const ViewInfo &);

    uint32_t count(const PipelineGroups) const override;
    void clear(const PipelineGroups) override;

  private:
    [[nodiscard]] rhi::GraphicsPipeline
    _createPipeline(const BaseGeometryPassInfo &) const;
  };

  class OutlinePass final : public rhi::RenderPass<OutlinePass>,
                            public Technique {
    friend class BasePass;

  public:
    explicit OutlinePass(rhi::RenderDevice &);

    uint32_t count(const PipelineGroups) const override;
    void clear(const PipelineGroups) override;

    [[nodiscard]] FrameGraphResource
    addPostProcessPass(FrameGraph &, const FrameGraphBlackboard &,
                       FrameGraphResource target);

  private:
    [[nodiscard]] rhi::GraphicsPipeline
    _createPipeline(const rhi::PixelFormat colorFormat) const;
  };

private:
  SilhouettePass m_silhouettePass;
  OutlinePass m_outlinePass;
};

} // namespace gfx
