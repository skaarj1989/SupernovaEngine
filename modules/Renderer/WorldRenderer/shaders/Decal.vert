#version 460 core

#include "VertexAttributes.glsl"

#include "Resources/FrameBlock.glsl"
#include "Resources/CameraBlock.glsl"

#include "Resources/InstanceBuffer.glsl"
#include "Resources/TransformBuffer.glsl"

#include "Lib/Math.glsl"

#include "VertexData.glsl"

void main() {
  const Instance instance = GET_INSTANCE();
  vs_out.materialId = instance.materialId;
  vs_out.flags = instance.flags;

  const mat4 modelMatrix = g_Transforms[instance.transformId];
  vs_out.invModelMatrix = inverse(modelMatrix);

  gl_Position = u_Camera.viewProjection * modelMatrix * vec4(a_Position, 1.0);
  gl_Position.z -= EPSILON;

  vs_out.fragPosClipSpace = gl_Position;
}
