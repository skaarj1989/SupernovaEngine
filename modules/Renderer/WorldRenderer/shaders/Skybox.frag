#version 460 core

layout(location = 0) in VertexOutput { vec3 eyeDirection; }
fs_in;

#if CUBEMAP
layout(set = 2, binding = 0) uniform samplerCube t_Skybox;
#else
layout(set = 2, binding = 0) uniform sampler2D t_Skybox;

#  include <Lib/SampleSphericalMap.glsl>
#endif

layout(location = 0) out vec3 FragColor;
void main() {
#if CUBEMAP
  FragColor = texture(t_Skybox, fs_in.eyeDirection).rgb;
#else
  const vec2 texCoord = sampleSphericalMap(fs_in.eyeDirection);
  FragColor = texture(t_Skybox, texCoord).rgb;
#endif
}
