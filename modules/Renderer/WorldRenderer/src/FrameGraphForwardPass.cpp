#include "FrameGraphForwardPass.hpp"
#include "FrameGraphCommon.hpp"

#include "FrameGraphData/DummyResources.hpp"
#include "FrameGraphData/Frame.hpp"
#include "FrameGraphData/Camera.hpp"
#include "FrameGraphData/Transforms.hpp"
#include "FrameGraphData/Skins.hpp"
#include "FrameGraphData/MaterialProperties.hpp"
#include "FrameGraphData/Lights.hpp"
#include "FrameGraphData/LightCulling.hpp"
#include "FrameGraphData/ShadowMap.hpp"
#include "FrameGraphData/GBuffer.hpp"
#include "FrameGraphData/SceneColor.hpp"
#include "FrameGraphData/BRDF.hpp"
#include "FrameGraphData/SkyLight.hpp"

namespace gfx {

void read(FrameGraph::Builder &builder, const FrameGraphBlackboard &blackboard,
          const FrameGraphResource instances) {
  // Vert+Frag.

  read(builder, blackboard.get<FrameData>());
  read(builder, blackboard.get<CameraData>());

  if (auto d = blackboard.try_get<MaterialPropertiesData>(); d) {
    read(builder, *d);
  }

  // Mesh:

  const auto &dummyResources = blackboard.get<DummyResourcesData>();
  readInstances(builder, instances, dummyResources);
  read(builder, blackboard.try_get<TransformData>(), dummyResources);
  read(builder, blackboard.try_get<SkinData>(), dummyResources);

  // Lighting:

  read(builder, blackboard.get<BRDF>());
  if (auto *d = blackboard.try_get<SkyLightData>(); d) {
    read(builder, *d);
  }
  read(builder, blackboard.get<LightsData>(), PipelineStage::FragmentShader);
  if (auto *d = blackboard.try_get<LightCullingData>(); d) {
    read(builder, *d);
  }
  read(builder, blackboard.get<ShadowMapData>(), dummyResources);

  readSceneDepth(builder, blackboard.get<GBufferData>().depth,
                 ReadFlags::Attachment | ReadFlags::Sampling);
  readSceneColor(builder, blackboard.get<SceneColorData>().HDR);
}

} // namespace gfx
