#include <Resources/CameraBlock.h.slang>

#include <Lib/Frustum.h.slang>

[[vk::binding(0, 2)]]
RWStructuredBuffer<Frustum> g_GridFrustums;

#include <Lib/Math.h.slang>
#include <Lib/SpaceTraversal.h.slang>

struct PushConstants {
  uint2 gridSize;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> u_PC;

// Example: Screen size = 1280x720, TILE_SIZE = 16:
// gl_NumWorkGroups = [5 x 3 x 1]
// gl_GlobalInvocationID/SV_DispatchThreadID = [0..79 x 0..47 x 0]
// Num invocations: 3840
// Num frustums: 3600

[numthreads(TILE_SIZE, TILE_SIZE, 1)]
void main(uint3 dispatchThreadID: SV_DispatchThreadID) {
  // Each thread represents a single tile/frustum
  const uint2 tileId = uint2(dispatchThreadID.xy);
  if (any(tileId >= u_PC.gridSize)) return;

#define TL 0 // top-left
#define TR 1 // top-right
#define BL 2 // bottom-left
#define BR 3 // bottom-right

  float2 screenPos[4];
  screenPos[TL] = float2(tileId.xy * TILE_SIZE);
  screenPos[TR] = float2(tileId.x + 1, tileId.y) * TILE_SIZE;
  screenPos[BL] = float2(tileId.x, tileId.y + 1) * TILE_SIZE;
  screenPos[BR] = float2(tileId.x + 1, tileId.y + 1) * TILE_SIZE;

  float3 viewPos[4];
  [[unroll]]
  for (uint i = 0; i < 4; ++i) {
    viewPos[i] = screenToView(screenPos[i], 0.0, 1.0);
  }

#define LEFT 0
#define RIGHT 1
#define TOP 2
#define BOTTOM 3

  const float3 kEyePos = float3(0.0);

  // All planes faces inwards
  Frustum frustum;
  frustum.planes[LEFT] = computePlane(kEyePos, viewPos[BL], viewPos[TL]);
  frustum.planes[RIGHT] = computePlane(kEyePos, viewPos[TR], viewPos[BR]);
  frustum.planes[TOP] = computePlane(kEyePos, viewPos[TL], viewPos[TR]);
  frustum.planes[BOTTOM] = computePlane(kEyePos, viewPos[BR], viewPos[BL]);

  const uint tileIndex = flatten2D(tileId, u_PC.gridSize.x);
  g_GridFrustums[tileIndex] = frustum;
}
