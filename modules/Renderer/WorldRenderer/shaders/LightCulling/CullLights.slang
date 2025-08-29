#define DEBUG_HEATMAP 1
#define DEBUG_DEPTH 2

struct PushConstants {
  uint2 numThreadGroups;
  uint numLights;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> u_PC;

#include <Resources/CameraBlock.h.slang>

[[vk::binding(0, 2)]]
Texture2D t_SceneDepth;

// [startOffset, lightCount]
// .rg = opaque geometry
// .ba = transparent geometry
[[vk::binding(3, 2), vk::image_format("rgba32ui")]]
RWTexture2D<uint4> i_LightGrid;

#include <Lib/Depth.h.slang>
#include <Lib/Light.h.slang>

[[vk::binding(3, 1)]]
StructuredBuffer<Light> g_LightBuffer;

#include <Lib/FrustumCulling.h.slang>

[[vk::binding(1, 2)]]
StructuredBuffer<Frustum> g_GridFrustums;

// .x = opaque geometry
// .y = transparent geometry
[[vk::binding(2, 2)]]
RWStructuredBuffer<uint2> g_LightIndexCounter;

// Indices to g_LightBuffer
// .x = opaque geometry
// .y = transparent geometry
[[vk::binding(4, 2)]]
RWStructuredBuffer<uint2> g_LightIndexList;

#if _DEBUG_OUTPUT
[[vk::binding(5, 2), vk::image_format("rgba8")]]
RWTexture2D i_DebugMap;

#  include <Lib/HeatMap.h.slang>
#endif

//
// Shared data (per group/tile):
//

struct TileData {
  uint minDepthInt;
  uint maxDepthInt;
  Frustum frustum;
};
groupshared TileData tileData;

#define MAX_NUM_LIGHTS_PER_TILE 1024

struct LightBucket {
  uint lightCount;
  uint startOffset;
  uint lightList[MAX_NUM_LIGHTS_PER_TILE];
};
groupshared LightBucket opaqueGeometry;
groupshared LightBucket transparentGeometry;

#define _APPEND_LIGHT(lightIndex, bucket)                                      \
  {                                                                            \
    uint index;                                                                \
    InterlockedAdd(bucket.lightCount, 1, index);                               \
    if (index < MAX_NUM_LIGHTS_PER_TILE) bucket.lightList[index] = lightIndex; \
  }

// Example: Screen size = 1280x720, TILE_SIZE = 16:
// gl_NumWorkGroups = [80 x 45 x 1] = 3600 frustums
// gl_LocalInvocationID/SV_GroupThreadID = [0..15 x 0..15 x 0]
// gl_LocalInvocationIndex/SV_GroupIndex = 16x16 = [0..255]
// gl_WorkGroupID/SV_GroupID = [0..79 x 0..44 x 0]
// gl_GlobalInvocationID/SV_DispatchThreadID = [0..1279 x 0..719 x 0]
// gl_WorkGroupSize = TILE_SIZE

[shader("compute") numthreads(TILE_SIZE, TILE_SIZE, 1)]
void main(uint3 dispatchThreadId: SV_DispatchThreadID,
          uint3 groupThreadId: SV_GroupThreadID, uint groupIndex: SV_GroupIndex,
          uint3 groupId: SV_GroupID) {
  if (all(dispatchThreadId == uint3(0))) {
    g_LightIndexCounter[0] = uint2(0);
  }
  GroupMemoryBarrierWithGroupSync();

  const uint2 tileId = uint2(groupId.xy);

  if (groupIndex == 0) {
    tileData.minDepthInt = 0xFFFFFFFF;
    tileData.maxDepthInt = 0;
    const uint tileIndex = flatten2D(tileId, u_PC.numThreadGroups.x);
    tileData.frustum = g_GridFrustums[tileIndex];

    opaqueGeometry.lightCount = 0;
    transparentGeometry.lightCount = 0;
  }

  GroupMemoryBarrierWithGroupSync();

  // Each thread indicates a single pixel
  const int2 screenCoord = int2(dispatchThreadId.xy);
  if (all(screenCoord < u_Camera.resolution)) {
    const float depth = fetchDepth(t_SceneDepth, screenCoord);

    const uint depthInt = asuint(depth);
    InterlockedMin(tileData.minDepthInt, depthInt);
    InterlockedMax(tileData.maxDepthInt, depthInt);
  }

  GroupMemoryBarrierWithGroupSync();

  const float minDepth = asfloat(tileData.minDepthInt);
  const float maxDepth = asfloat(tileData.maxDepthInt);

  const float minDepthVS = screenToView(float2(0.0), minDepth, 1.0).z;
  const float maxDepthVS = screenToView(float2(0.0), maxDepth, 1.0).z;
  const float nearClipVS = screenToView(float2(0.0), NEAR_CLIP_PLANE, 1.0).z;

  const Plane minPlane = { float3(0.0, 0.0, -1.0), minDepthVS };

  const uint kStride = TILE_SIZE * TILE_SIZE;
  for (uint i = groupIndex; i < u_PC.numLights; i += kStride) {
    const Light light = g_LightBuffer[i];
    const float3 lightPos =
      mul(u_Camera.view, float4(light.position.xyz, 1.0)).xyz;
    const float3 lightDirection =
      mul(u_Camera.view, float4(light.direction.xyz, 0.0)).xyz;
    const float lightRange = _getLightRange(light);

    switch (light.type) {
    case LightType::Directional: {
      _APPEND_LIGHT(i, opaqueGeometry);
      _APPEND_LIGHT(i, transparentGeometry);
    } break;
    case LightType::Spot: {
      // https://simoncoenen.com/blog/programming/graphics/SpotlightCulling
#if 1
      const float halfAngleCos = cos(light.outerConeAngle * 0.5);
      const float sphereRadius =
        lightRange * 0.5 / (halfAngleCos * halfAngleCos);
      const float3 sphereCenter = lightPos + lightDirection * sphereRadius;
      const Sphere sphere = { sphereCenter, sphereRadius };
      if (sphereInsideFrustum(sphere, tileData.frustum, nearClipVS,
                              maxDepthVS)) {
        _APPEND_LIGHT(i, transparentGeometry);
        if (!sphereBehindPlane(sphere, minPlane)) {
          _APPEND_LIGHT(i, opaqueGeometry);
        }
      }
#else
      // FIXME: accuracy problems, corrupted visuals!
      const float coneRadius = tan(light.outerConeAngle) * lightRange;
      const Cone cone = { lightPos, lightRange, lightDirection, coneRadius };
      if (coneInsideFrustum(cone, tileData.frustum, nearClipVS, maxDepthVS)) {
        _APPEND_LIGHT(i, transparentGeometry);
        if (!coneBehindPlane(cone, minPlane)) {
          _APPEND_LIGHT(i, opaqueGeometry);
        }
      }
#endif
    } break;
    case LightType::Point: {
      const Sphere sphere = { lightPos, lightRange };
      if (sphereInsideFrustum(sphere, tileData.frustum, nearClipVS,
                              maxDepthVS)) {
        _APPEND_LIGHT(i, transparentGeometry);
        if (!sphereBehindPlane(sphere, minPlane)) {
          _APPEND_LIGHT(i, opaqueGeometry);
        }
      }
    } break;
    }
  }

  GroupMemoryBarrierWithGroupSync();

  //
  // Update global memory with visible light buffer:
  //

  if (groupIndex == 0) {
    InterlockedAdd(g_LightIndexCounter[0].x, opaqueGeometry.lightCount,
                   opaqueGeometry.startOffset);
    InterlockedAdd(g_LightIndexCounter[0].y, transparentGeometry.lightCount,
                   transparentGeometry.startOffset);

    i_LightGrid[tileId] =
      uint4(opaqueGeometry.startOffset, opaqueGeometry.lightCount,
            transparentGeometry.startOffset, transparentGeometry.lightCount);
  }

  GroupMemoryBarrierWithGroupSync();

  for (uint i = groupIndex; i < opaqueGeometry.lightCount; i += kStride) {
    g_LightIndexList[opaqueGeometry.startOffset + i].x =
      opaqueGeometry.lightList[i];
  }
  for (uint i = groupIndex; i < transparentGeometry.lightCount; i += kStride) {
    g_LightIndexList[transparentGeometry.startOffset + i].y =
      transparentGeometry.lightList[i];
  }

  //
  // Debug output:
  //

#if _DEBUG_OUTPUT
  const float kOverlayOpacity = 0.35;
  const float3 kBorderColor = float3(0.0);
  // Borders
  if (groupThreadId.x == 0 || groupThreadId.y == 0) {
    i_DebugMap[screenCoord] = float4(kBorderColor, 0.9);
  } else if (groupThreadId.x < 1 || groupThreadId.y < 1) {
    i_DebugMap[screenCoord] = float4(kBorderColor, 0.5);
  } else {
#  if _DEBUG_OUTPUT == DEBUG_HEATMAP
    // Light heatmap for opaque geometry
    if (opaqueGeometry.lightCount > 0) {
      const float3 heat = calculateHeat(0.0, 50.0, opaqueGeometry.lightCount);
      i_DebugMap[screenCoord] = float4(heat, kOverlayOpacity);
    } else {
      const float3 kNoLightsInTileColor = float3(0.0);
      i_DebugMap[screenCoord] = float4(kNoLightsInTileColor, kOverlayOpacity);
    }
#  elif _DEBUG_OUTPUT == DEBUG_DEPTH
    const float averageDepth =
      lerp(linearizeDepth(minDepth) / u_Camera.far,
           linearizeDepth(maxDepth) / u_Camera.far, 0.5);
    i_DebugMap[screenCoord] = float4(float3(averageDepth), kOverlayOpacity);
#  endif
  }
#endif
}
