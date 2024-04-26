#pragma once

#include "fg/FrameGraph.hpp"
#include "fg/Blackboard.hpp"
#include "PipelineStage.hpp"
#include "GBufferFlags.hpp"
#include <optional>

namespace gfx {

struct LightingPassFeatures;

void getLightingPassFeatures(LightingPassFeatures &,
                             const FrameGraphBlackboard &);

// ---

struct FrameData;
void read(FrameGraph::Builder &, const FrameData &,
          const PipelineStage = PipelineStage::VertexShader |
                                PipelineStage::FragmentShader);

struct CameraData;
void read(FrameGraph::Builder &, const CameraData &,
          const PipelineStage = PipelineStage::VertexShader |
                                PipelineStage::FragmentShader);

struct DummyResourcesData;
void readInstances(FrameGraph::Builder &,
                   const std::optional<FrameGraphResource> instances,
                   const DummyResourcesData &);
struct TransformData;
void read(FrameGraph::Builder &, const TransformData *,
          const DummyResourcesData &);
struct SkinData;
void read(FrameGraph::Builder &, const SkinData *, const DummyResourcesData &);

struct MaterialPropertiesData;
void read(FrameGraph::Builder &, const MaterialPropertiesData &);

struct GBufferData;
void read(FrameGraph::Builder &, const GBufferData &, const GBufferFlags);

struct BRDF;
void read(FrameGraph::Builder &, const BRDF &);
struct SkyLightData;
void read(FrameGraph::Builder &, const SkyLightData &);

struct LightsData;
void read(FrameGraph::Builder &, const LightsData &, const PipelineStage);
struct LightCullingData;
void read(FrameGraph::Builder &, const LightCullingData &);

struct ShadowMapData;
void read(FrameGraph::Builder &, const ShadowMapData &,
          const DummyResourcesData &);
void readShadowBlock(FrameGraph::Builder &,
                     const FrameGraphResource shadowBlock);

struct SSAOData;
void read(FrameGraph::Builder &, const SSAOData &);

struct GlobalIlluminationData;
void read(FrameGraph::Builder &, const GlobalIlluminationData &);
void readSceneGrid(FrameGraph::Builder &,
                   const FrameGraphResource sceneGridBlock,
                   const PipelineStage);

void readSceneColor(FrameGraph::Builder &, const FrameGraphResource sceneColor);

enum class ReadFlags {
  Attachment = 1 << 1,
  Sampling = 1 << 2,
};
void readSceneDepth(FrameGraph::Builder &, const FrameGraphResource sceneDepth,
                    const ReadFlags);

void writeUserData(FrameGraph::Builder &, FrameGraphBlackboard &);

} // namespace gfx

template <> struct has_flags<gfx::ReadFlags> : std::true_type {};
