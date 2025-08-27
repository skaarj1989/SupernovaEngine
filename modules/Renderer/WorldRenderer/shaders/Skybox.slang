//
// Vertex Shader:
//

#include "Resources/CameraBlock.h.slang"

#include "Lib/SpaceTraversal.h.slang"
#include "FullScreenTriangle.h.slang"

struct VSOutput {
  float4 position : SV_Position;
  float3 eyeDirection;
};

[shader("vertex")]
VSOutput vertexMain(uint vertexIndex: SV_VulkanVertexID) {
  VSOutput output;

  emitVertex(vertexIndex, output.position);
  output.position = output.position.xyww;

  const float3 P = NDCToView(output.position);
  output.eyeDirection = viewToWorld(float4(P, 0.0));

  return output;
}

//
// Fragment Shader:
//

#include "Lib/SampleSphericalMap.h.slang"

#if CUBEMAP
[[vk::binding(0, 2)]]
SamplerCube t_Skybox : register(t0);
#else
[[vk::binding(0, 2)]]
Sampler2D t_Skybox : register(t0);
#endif

[shader("fragment")]
float3 fragmentMain(VSOutput input) : SV_Target0 {
#if CUBEMAP
  return t_Skybox.Sample(input.eyeDirection).rgb;
#else
  const float2 texCoord = sampleSphericalMap(input.eyeDirection);
  return t_Skybox.Sample(texCoord, 0).rgb;
#endif
}
