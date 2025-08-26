//
// Vertex Shader:
//

#include "Resources/CameraBlock.slang"

struct Instance {
  float4x4 modelMatrix;
  float4 color;
};
#include "Resources/InstanceBuffer.slang"

struct VSInput {
  [[vk::location(0)]]
  float4 position;
  [[vk::location(1)]]
  float4 color;
};

struct VSOutput {
  float4 position : SV_Position;
  float4 color : COLOR0;
  float pointSize : SV_PointSize;
};

[shader("vertex")]
VSOutput vertexMain(VSInput input, uint instanceID: SV_VulkanInstanceID) {
  VSOutput output;
  output.color = input.color;
#if TRIANGLE_MESH
  const Instance instance = g_Instances[u_PC.instanceOffset + instanceID];
  output.color *= instance.color;
  output.position =
    mul(u_Camera.viewProjection,
        mul(instance.modelMatrix, float4(input.position.xyz, 1.0)));
#else
  output.position =
    mul(u_Camera.viewProjection, float4(input.position.xyz, 1.0));
  output.pointSize = input.position.w;
#endif
  return output;
}

//
// Fragment Shader:
//

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target { return input.color; }
