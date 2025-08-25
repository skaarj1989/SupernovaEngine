//
// Vertex Shader:
//

struct PushConstant {
#if USE_PROJECTION_MATRIX
  float4x4 projectionMatrix;
#else
  float2 scale;
  float2 translate;
#endif
};

[[vk::push_constant]]
ConstantBuffer<PushConstant> u_PC;

struct VSInput {
  [[vk::location(0)]]
  float2 position : POSITION;
  [[vk::location(1)]]
  float2 texCoord : TEXCOORD;
  [[vk::location(2)]]
  float4 color : COLOR;
};

struct VSOutput {
  float4 position : SV_Position;
  float2 texCoord : TEXCOORD0;
  float4 color : COLOR0;
};

[shader("vertex")]
VSOutput vertexMain(VSInput input) {
  VSOutput output;
#if USE_PROJECTION_MATRIX
  output.position =
    mul(u_PC.projectionMatrix, float4(input.position, 0.0, 1.0));
#else
  output.position =
    float4(input.position * u_PC.scale + u_PC.translate, 0.0, 1.0);
#endif
  output.texCoord = input.texCoord;
  output.color = input.color;
  return output;
}

//
// Fragment Shader:
//

#if CUBEMAP
[[vk::binding(0, 0)]]
SamplerCube : register(t0);

#  define _EQUIRECTANGULAR 1
#  define _CUBE_NET 2

#  define POSITIVE_X 0
#  define NEGATIVE_X 1
#  define POSITIVE_Y 2
#  define NEGATIVE_Y 3
#  define POSITIVE_Z 4
#  define NEGATIVE_Z 5

float3 getCubeFace(float2 uv, uint face) {
  switch (face) {
  case POSITIVE_X:
    return float3(1.0, -uv.y, uv.x);
  case NEGATIVE_X:
    return float3(-1.0, -uv.y, uv.x);
  case POSITIVE_Y:
    return float3(uv.x, 1.0, uv.y);
  case NEGATIVE_Y:
    return float3(uv.x, -1.0, -uv.y);
  case POSITIVE_Z:
    return float3(uv.x, -uv.y, 1.0);
  case NEGATIVE_Z:
    return float3(-uv.x, -uv.y, -1.0);
  }
  return float3(0.0, 0.0, 0.0);
}

float3 getCubeCoord(float2 uv) {
#  if MODE == _EQUIRECTANGULAR
  const kPI = 3.1415;
  const phi = uv.x * kPI * 2.0;
  const theta = (-uv.y + 0.5) * kPI;
  return float3(cos(phi) * cos(theta), sin(theta), sin(phi) * cos(theta));
#  elif MODE == _CUBE_NET
  const kNetSize = float2(4.0, 3.0);
  const kInvNetSize = 1.0 / kNetSize;

  const coord = int2(floor(uv / kInvNetSize));
  const offset = kInvNetSize * float2(coord);

  uv = (uv - offset) * kNetSize; // [-1, 1]
  uv = uv * 2.0 - 1.0;           // [ 0, 1]

  if (all(coord == int2(3, 1)))
    return getCubeFace(uv, POSITIVE_X);
  else if (all(coord == int2(1, 1)))
    return getCubeFace(uv, NEGATIVE_X);
  else if (all(coord == int2(2, 0)))
    return getCubeFace(uv, POSITIVE_Y);
  else if (all(coord == int2(2, 2)))
    return getCubeFace(uv, NEGATIVE_Y);
  else if (all(coord == int2(2, 1)))
    return getCubeFace(uv, POSITIVE_Z);
  else if (all(coord == int2(0, 1)))
    return getCubeFace(uv, NEGATIVE_Z);

  discard;
#  endif
  return float3(0.0, 0.0, 0.0);
}
#else
[[vk::binding(0, 0)]]
Sampler2D t_0 : register(t0);
#endif

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target {
#if CUBEMAP
  return input.color * t_0.Sample(getCubeCoord(input.texCoord));
#else
#  if USE_MONOCHROMATIC_FONT
  return input.color * t_0.Sample(input.texCoord).rrrr;
#  else
  return input.color * t_0.Sample(input.texCoord);
#  endif
#endif
}
