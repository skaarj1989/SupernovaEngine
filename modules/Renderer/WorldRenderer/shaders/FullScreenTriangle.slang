#pragma once

struct VSOutput {
  float4 position : SV_Position;
  float2 texCoord;
};

#include "FullScreenTriangle.h.slang"

[shader("vertex")]
VSOutput vertexMain(uint vertexIndex: SV_VulkanVertexID) {
  VSOutput output;
  const float2 texCoord = emitVertex(vertexIndex, output.position);
  output.texCoord = texCoord;
  output.position = float4(texCoord * 2.0 - 1.0, 0.0, 1.0);
  return output;
}
