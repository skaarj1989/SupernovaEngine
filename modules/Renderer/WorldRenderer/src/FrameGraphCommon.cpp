#include "FrameGraphCommon.hpp"

#include "FrameGraphResourceAccess.hpp"
#include "LightingPassFeatures.hpp"

#include "FrameGraphData/DummyResources.hpp"
#include "FrameGraphData/Frame.hpp"
#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/Transforms.hpp"
#include "FrameGraphData/Skins.hpp"
#include "FrameGraphData/MaterialProperties.hpp"
#include "FrameGraphData/GBuffer.hpp"
#include "FrameGraphData/BRDF.hpp"
#include "FrameGraphData/SkyLight.hpp"
#include "FrameGraphData/Lights.hpp"
#include "FrameGraphData/LightCulling.hpp"
#include "FrameGraphData/ShadowMap.hpp"
#include "FrameGraphData/SSAO.hpp"
#include "FrameGraphData/GlobalIllumination.hpp"

#include <utility> // to_underlying

namespace gfx {

void getLightingPassFeatures(LightingPassFeatures &features,
                             const FrameGraphBlackboard &blackboard) {
  if (auto *d = blackboard.try_get<LightCullingData>(); d)
    features.tileSize = d->tileSize;

  features.ssao = blackboard.has<SSAOData>();
  features.globalIllumination = blackboard.has<GlobalIlluminationData>();
  features.skyLight = blackboard.has<SkyLightData>();
}

void read(FrameGraph::Builder &builder, const FrameData &data,
          const PipelineStage pipelineStage) {
  builder.read(data.frameBlock, BindingInfo{
                                  .location = {.set = 0, .binding = 0},
                                  .pipelineStage = pipelineStage,
                                });
}
void read(FrameGraph::Builder &builder, const CameraData &data,
          const PipelineStage pipelineStage) {
  builder.read(data.cameraBlock, BindingInfo{
                                   .location = {.set = 1, .binding = 0},
                                   .pipelineStage = pipelineStage,
                                 });
}

void readInstances(FrameGraph::Builder &builder,
                   const std::optional<FrameGraphResource> instances,
                   const DummyResourcesData &dummyResources) {
  builder.read(instances.value_or(dummyResources.storageBuffer),
               BindingInfo{
                 .location = {.set = 1, .binding = 2},
                 .pipelineStage = PipelineStage::VertexShader,
               });
}
void read(FrameGraph::Builder &builder, const TransformData *data,
          const DummyResourcesData &dummyResources) {
  builder.read(data ? data->transforms : dummyResources.storageBuffer,
               BindingInfo{
                 .location = {.set = 1, .binding = 1},
                 .pipelineStage = PipelineStage::VertexShader,
               });
}
void read(FrameGraph::Builder &builder, const SkinData *data,
          const DummyResourcesData &dummyResources) {
  builder.read(data ? data->skins : dummyResources.storageBuffer,
               BindingInfo{
                 .location = {.set = 0, .binding = 2},
                 .pipelineStage = PipelineStage::VertexShader,
               });
}

void read(FrameGraph::Builder &builder, const MaterialPropertiesData &data) {
  builder.read(data.properties,
               BindingInfo{
                 .location = {.set = 0, .binding = 1},
                 .pipelineStage =
                   PipelineStage::VertexShader | PipelineStage::FragmentShader,
               });
}

void read(FrameGraph::Builder &builder, const GBufferData &data,
          const GBufferFlags flags) {
  if (bool(flags & GBufferFlags::Depth)) {
    builder.read(data.depth,
                 TextureRead{
                   .binding =
                     {
                       .location = {.set = 1, .binding = 5},
                       .pipelineStage = PipelineStage::FragmentShader,
                     },
                   .type = TextureRead::Type::SampledImage,
                 });
  }
  if (bool(flags & GBufferFlags::Normal)) {
    builder.read(data.normal,
                 TextureRead{
                   .binding =
                     {
                       .location = {.set = 1, .binding = 6},
                       .pipelineStage = PipelineStage::FragmentShader,
                     },
                   .type = TextureRead::Type::SampledImage,
                 });
  }
  if (bool(flags & GBufferFlags::Emissive)) {
    builder.read(data.emissive,
                 TextureRead{
                   .binding =
                     {
                       .location = {.set = 1, .binding = 7},
                       .pipelineStage = PipelineStage::FragmentShader,
                     },
                   .type = TextureRead::Type::SampledImage,
                 });
  }
  if (bool(flags & GBufferFlags::AlbedoSpecular)) {
    builder.read(data.albedo,
                 TextureRead{
                   .binding =
                     {
                       .location = {.set = 1, .binding = 8},
                       .pipelineStage = PipelineStage::FragmentShader,
                     },
                   .type = TextureRead::Type::SampledImage,
                 });
  }
  if (bool(flags & GBufferFlags::MetallicRoughnessAO)) {
    builder.read(data.metallicRoughnessAO,
                 TextureRead{
                   .binding =
                     {
                       .location = {.set = 1, .binding = 9},
                       .pipelineStage = PipelineStage::FragmentShader,
                     },
                   .type = TextureRead::Type::SampledImage,
                 });
  }
  if (bool(flags & GBufferFlags::Misc)) {
    builder.read(data.misc,
                 TextureRead{
                   .binding =
                     {
                       .location = {.set = 1, .binding = 10},
                       .pipelineStage = PipelineStage::FragmentShader,
                     },
                   .type = TextureRead::Type::SampledImage,
                 });
  }
}

void read(FrameGraph::Builder &builder, const BRDF &data) {
  builder.read(data.lut, TextureRead{
                           .binding =
                             {
                               .location = {.set = 2, .binding = 0},
                               .pipelineStage = PipelineStage::FragmentShader,
                             },
                           .type = TextureRead::Type::CombinedImageSampler,
                         });
}
void read(FrameGraph::Builder &builder, const SkyLightData &data) {
  builder.read(data.diffuse,
               TextureRead{
                 .binding =
                   {
                     .location = {.set = 2, .binding = 1},
                     .pipelineStage = PipelineStage::FragmentShader,
                   },
                 .type = TextureRead::Type::CombinedImageSampler,
               });
  builder.read(data.specular,
               TextureRead{
                 .binding =
                   {
                     .location = {.set = 2, .binding = 2},
                     .pipelineStage = PipelineStage::FragmentShader,
                   },
                 .type = TextureRead::Type::CombinedImageSampler,
               });
}

void read(FrameGraph::Builder &builder, const LightsData &data,
          const PipelineStage pipelineStage) {
  builder.read(data.lights, BindingInfo{
                              .location = {.set = 1, .binding = 3},
                              .pipelineStage = pipelineStage,
                            });
}
void read(FrameGraph::Builder &builder, const LightCullingData &data) {
  builder.read(data.lightGrid,
               TextureRead{
                 .binding =
                   {
                     .location = {.set = 2, .binding = 7},
                     .pipelineStage = PipelineStage::FragmentShader,
                   },
                 .type = TextureRead::Type::StorageImage,
               });
  builder.read(data.lightIndices,
               BindingInfo{
                 .location = {.set = 2, .binding = 8},
                 .pipelineStage = PipelineStage::FragmentShader,
               });
}

void read(FrameGraph::Builder &builder, const ShadowMapData &data,
          const DummyResourcesData &dummyResources) {
  readShadowBlock(builder,
                  data.shadowBlock.value_or(dummyResources.uniformBuffer));
  builder.read(
    data.cascadedShadowMaps.value_or(dummyResources.shadowMaps2DArray),
    TextureRead{
      .binding =
        {
          .location = {.set = 2, .binding = 3},
          .pipelineStage = PipelineStage::FragmentShader,
        },
      .type = TextureRead::Type::SampledImage,
    });
  builder.read(
    data.spotLightShadowMaps.value_or(dummyResources.shadowMaps2DArray),
    TextureRead{
      .binding =
        {
          .location = {.set = 2, .binding = 4},
          .pipelineStage = PipelineStage::FragmentShader,
        },
      .type = TextureRead::Type::SampledImage,
    });
  builder.read(data.omniShadowMaps.value_or(dummyResources.shadowMapsCubeArray),
               TextureRead{
                 .binding =
                   {
                     .location = {.set = 2, .binding = 5},
                     .pipelineStage = PipelineStage::FragmentShader,
                   },
                 .type = TextureRead::Type::SampledImage,
               });
}
void readShadowBlock(FrameGraph::Builder &builder,
                     const FrameGraphResource shadowBlock) {
  builder.read(shadowBlock, BindingInfo{
                              .location = {.set = 1, .binding = 4},
                              .pipelineStage = PipelineStage::FragmentShader,
                            });
}

void read(FrameGraph::Builder &builder, const SSAOData &data) {
  builder.read(data.ssao, TextureRead{
                            .binding =
                              {
                                .location = {.set = 2, .binding = 6},
                                .pipelineStage = PipelineStage::FragmentShader,
                              },
                            .type = TextureRead::Type::SampledImage,
                          });
}

void read(FrameGraph::Builder &builder, const GlobalIlluminationData &data) {
  readSceneGrid(builder, data.sceneGridBlock, PipelineStage::FragmentShader);
  builder.read(data.LPV.r, TextureRead{
                             .binding =
                               {
                                 .location = {.set = 2, .binding = 10},
                                 .pipelineStage = PipelineStage::FragmentShader,
                               },
                             .type = TextureRead::Type::SampledImage,
                           });
  builder.read(data.LPV.g, TextureRead{
                             .binding =
                               {
                                 .location = {.set = 2, .binding = 11},
                                 .pipelineStage = PipelineStage::FragmentShader,
                               },
                             .type = TextureRead::Type::SampledImage,
                           });
  builder.read(data.LPV.b, TextureRead{
                             .binding =
                               {
                                 .location = {.set = 2, .binding = 12},
                                 .pipelineStage = PipelineStage::FragmentShader,
                               },
                             .type = TextureRead::Type::SampledImage,
                           });
}
void readSceneGrid(FrameGraph::Builder &builder,
                   const FrameGraphResource sceneGridBlock,
                   const PipelineStage pipelineStage) {
  builder.read(sceneGridBlock, BindingInfo{
                                 .location = {.set = 2, .binding = 9},
                                 .pipelineStage = pipelineStage,
                               });
}

void readSceneColor(FrameGraph::Builder &builder,
                    const FrameGraphResource sceneColor) {
  builder.read(sceneColor, TextureRead{
                             .binding =
                               {
                                 .location = {.set = 1, .binding = 11},
                                 .pipelineStage = PipelineStage::FragmentShader,
                               },
                             .type = TextureRead::Type::CombinedImageSampler,
                           });
}
void readSceneDepth(FrameGraph::Builder &builder,
                    const FrameGraphResource sceneDepth,
                    const ReadFlags readFlags) {
  assert(std::to_underlying(readFlags) > 0);

  // The order is important!
  // Texture sampling is placed lower in pipeline stage (effective barrier).

  if (bool(readFlags & ReadFlags::Attachment)) {
    builder.read(sceneDepth, Attachment{});
  }
  if (bool(readFlags & ReadFlags::Sampling)) {
    builder.read(sceneDepth,
                 TextureRead{
                   .binding =
                     {
                       .location = {.set = 1, .binding = 5},
                       .pipelineStage = PipelineStage::FragmentShader,
                     },
                   .type = TextureRead::Type::CombinedImageSampler,
                 });
  }
}

} // namespace gfx
