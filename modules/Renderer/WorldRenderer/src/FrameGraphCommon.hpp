#pragma once

#include "fg/FrameGraph.hpp"
#include "fg/Blackboard.hpp"
#include "PipelineStage.hpp"
#include "GBufferFlags.hpp"
#include "renderer/LightingPassFeatures.hpp"

namespace gfx {

void getLightingPassFeatures(LightingPassFeatures &,
                             const FrameGraphBlackboard &);

// ---

struct FrameData;
void read(FrameGraph::Builder &, const FrameData &,
          PipelineStage = PipelineStage::VertexShader |
                          PipelineStage::FragmentShader);

struct CameraData;
void read(FrameGraph::Builder &, const CameraData &,
          PipelineStage = PipelineStage::VertexShader |
                          PipelineStage::FragmentShader);

struct DummyResourcesData;
void readInstances(FrameGraph::Builder &,
                   std::optional<FrameGraphResource> instances,
                   const DummyResourcesData &);
struct TransformData;
void read(FrameGraph::Builder &, const TransformData *,
          const DummyResourcesData &);
struct SkinData;
void read(FrameGraph::Builder &, const SkinData *, const DummyResourcesData &);

struct MaterialPropertiesData;
void read(FrameGraph::Builder &, const MaterialPropertiesData &);

struct GBufferData;
void read(FrameGraph::Builder &, const GBufferData &, GBufferFlags);

struct BRDF;
void read(FrameGraph::Builder &, const BRDF &);
struct SkyLightData;
void read(FrameGraph::Builder &, const SkyLightData &);

struct LightsData;
void read(FrameGraph::Builder &, const LightsData &, PipelineStage);
struct LightCullingData;
void read(FrameGraph::Builder &, const LightCullingData &);

struct ShadowMapData;
void read(FrameGraph::Builder &, const ShadowMapData &,
          const DummyResourcesData &);
void readShadowBlock(FrameGraph::Builder &, FrameGraphResource shadowBlock);

struct SSAOData;
void read(FrameGraph::Builder &, const SSAOData &);

struct GlobalIlluminationData;
void read(FrameGraph::Builder &, const GlobalIlluminationData &);
void readSceneGrid(FrameGraph::Builder &, FrameGraphResource sceneGridBlock,
                   PipelineStage);

void readSceneColor(FrameGraph::Builder &, FrameGraphResource sceneColor);

enum class ReadFlags {
  Attachment = 1 << 1,
  Sampling = 1 << 2,
};

void readSceneDepth(FrameGraph::Builder &, FrameGraphResource sceneDepth,
                    ReadFlags);

} // namespace gfx

template <> struct has_flags<gfx::ReadFlags> : std::true_type {};
