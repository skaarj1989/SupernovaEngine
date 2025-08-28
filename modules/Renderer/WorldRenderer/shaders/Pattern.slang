//
// Vertex Shader:
//

#include "FullScreenTriangle.slang"

//
// Fragment Shader:
//

[shader("fragment")]
float4 fragmentMain(VSOutput input) : SV_Target {
  float4 fragColor = float4(1.0, 0.0, 1.0, 1.0);
  // https://github.com/KhronosGroup/glTF-Sample-Viewer/commit/9da7b3aca927a57a0abe4b9a32d565c6d72e5a4d
  fragColor.rb = float2(
    max(2.0 * sin(0.1 * (input.position.x + input.position.y)), 0.0) + 0.3);
  return fragColor;
}
