#include "Resources/CameraBlock.h.slang"

//
// Vertex Shader:
//

#include "FullScreenTriangle.h.slang"
#include "Lib/SpaceTraversal.h.slang"

struct VSOutput {
  float4 position : SV_Position;
  float3 near;
  float3 far;
};

[shader("vertex")]
VSOutput vertexMain(uint vertexIndex: SV_VulkanVertexID) {
  VSOutput output;
  const float2 texCoord = emitVertex(vertexIndex, output.position);
  const float2 P = texCoord * 2.0 - 1.0;
  output.near = NDCToWorld(float4(P, 0.0, 1.0));
  output.far = NDCToWorld(float4(P, 1.0, 1.0));
  return output;
}

//
// Fragment Shader:
//

#include "Resources/CameraBlock.h.slang"
#include "Lib/Depth.h.slang"

// http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/

static const float3 kLineColor = float3(0.2);

float4 grid(float3 fragPos, float scale, float thickness) {
  const float2 coords = fragPos.xz * scale;
  const float2 fd = fwidth(coords);
  const float2 lattice = (abs(fract(coords - 0.5) - 0.5) / fd);
  const float line = min(lattice.x, lattice.y);

  float4 color = float4(kLineColor, 1.0 - min(line, 1.0));

  // Z axis
  const float minX = min(fd.x, 1.0);
  if (fragPos.x > minX * -0.1 && fragPos.x < minX * 0.1) {
    color = float4(0.0, 0.0, 1.0, 1.0);
  }
  // X axis
  const float minZ = min(fd.y, 1.0);
  if (fragPos.z > minZ * -0.1 && fragPos.z < minZ * 0.1) {
    color = float4(1.0, 0.0, 0.0, 1.0);
  }
  return color;
}

struct FSOutput {
  float4 color : SV_Target0;
  float depth : SV_Depth;
};

[shader("fragment")]
FSOutput main(VSOutput input) {
  const float t = -input.near.y / (input.far.y - input.near.y);
  const float3 fragPos = input.near + t * (input.far - input.near);

  const float3 ndc = worldToNDC(float4(fragPos, 1.0));
  if (ndc.z >= 1.0) discard;

  FSOutput output;
  output.depth = ndc.z - EPSILON;
  output.color =
    (grid(fragPos, 1.0, 0.1) + grid(fragPos, 10.0, 1.5)) * float(t > 0.0);

  const float linearDepth = linearizeDepth(ndc.z) / u_Camera.far;
  output.color.a *= max(0.0, (0.5 - linearDepth));
  return output;
}
