#pragma once

// https://www.saschawillems.de/blog/2016/08/13/vulkan-tutorial-on-rendering-a-fullscreen-quad-without-buffers/

// vkCmdDraw(cb, 3, 1, 0, 0);
float2 emitVertex(uint vertexIndex: SV_VulkanVertexID,
                  out float4 outPosition: SV_Position) {
  // Vertices in clockwise order (texCoord origin in upper left)
  const float2 texCoord = float2((vertexIndex << 1) & 2, vertexIndex & 2);
  outPosition = float4(texCoord * 2.0 - 1.0, 0.0, 1.0);
  return texCoord;
}
