#version 460 core

// http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/

#include "Resources/CameraBlock.glsl"
#include "Lib/Depth.glsl"

layout(location = 0) in VertexData {
  vec3 near;
  vec3 far;
}
fs_in;

const vec3 kLineColor = vec3(0.2);

vec4 grid(vec3 fragPos, float scale, float thickness) {
  const vec2 coords = fragPos.xz * scale;
  const vec2 fd = fwidth(coords);
  const vec2 lattice = (abs(fract(coords - 0.5) - 0.5) / fd);
  const float line = min(lattice.x, lattice.y);

  vec4 color = vec4(kLineColor, 1.0 - min(line, 1.0));

  // Z axis
  const float minX = min(fd.x, 1.0);
  if (fragPos.x > minX * -0.1 && fragPos.x < minX * 0.1) {
    color = vec4(0.0, 0.0, 1.0, 1.0);
  }
  // X axis
  const float minZ = min(fd.y, 1.0);
  if (fragPos.z > minZ * -0.1 && fragPos.z < minZ * 0.1) {
    color = vec4(1.0, 0.0, 0.0, 1.0);
  }
  return color;
}

layout(location = 0) out vec4 FragColor;
void main() {
  const float t = -fs_in.near.y / (fs_in.far.y - fs_in.near.y);
  const vec3 fragPos = fs_in.near + t * (fs_in.far - fs_in.near);

  const vec3 ndc = vec3(worldToNDC(vec4(fragPos, 1.0)));
  if (ndc.z >= 1.0) discard;

  gl_FragDepth = ndc.z - EPSILON;

  FragColor =
    (grid(fragPos, 1.0, 0.1) + grid(fragPos, 10.0, 1.5)) * float(t > 0.0);

  const float linearDepth = linearizeDepth(ndc.z) / u_Camera.far;
  FragColor.a *= max(0.0, (0.5 - linearDepth));
}
