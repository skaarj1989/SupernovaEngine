//
// Vertex Shader:
//

struct PushConstant {
  float4x4 transform;
  float2 translate;
};

[[vk::push_constant]]
ConstantBuffer<PushConstant> u_PC;

struct VSInput {
  [[vk::location(0)]]
  float2 position : POSITION;
  [[vk::location(1)]]
  float4 color : COLOR;
  [[vk::location(2)]]
  float2 texCoord : TEXCOORD;
};

struct VSOutput {
  float4 position : SV_Position;
  float4 color : COLOR0;
  float2 texCoord : TEXCOORD0;
};

[shader("vertex")]
VSOutput vertexMain(VSInput input) {
  VSOutput output;
  output.position =
    mul(u_PC.transform, float4(input.position + u_PC.translate.xy, 0.0, 1.0));
  output.color = input.color;
  output.texCoord = input.texCoord;
  return output;
}

//
// Fragment Shader:
//

[[vk::binding(0, 0)]]
Sampler2D t_0 : register(t0);

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target {
#if HAS_TEXTURE
  return input.color * t_0.Sample(input.texCoord);
#else
  return input.color;
#endif
}
