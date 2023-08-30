#version 460 core

#define _EQUIRECTANGULAR 1
#define _CUBE_NET 2

layout(location = 0) in VertexData {
  vec2 texCoord;
  vec4 color;
}
fs_in;

#if CUBEMAP
layout(binding = 0) uniform samplerCube t_0;

#  define POSITIVE_X 0
#  define NEGATIVE_X 1
#  define POSITIVE_Y 2
#  define NEGATIVE_Y 3
#  define POSITIVE_Z 4
#  define NEGATIVE_Z 5

vec3 getCubeFace(vec2 uv, uint face) {
  // clang-format off
  switch (face) {
    case POSITIVE_X: return vec3(1.0, -uv.yx);
    case NEGATIVE_X: return vec3(-1.0, -uv.y, uv.x);
    case POSITIVE_Y: return vec3(uv.x, 1.0, uv.y);
    case NEGATIVE_Y: return vec3(uv.x, -1.0, -uv.y);
    case POSITIVE_Z: return vec3(uv.x, -uv.y, 1.0);
    case NEGATIVE_Z: return vec3(-uv.xy, -1.0);
  }
  // clang-format on
  return vec3(0.0);
}

vec3 getCubeCoord(vec2 uv) {
#  if MODE == _EQUIRECTANGULAR
  const float kPI = 3.1415;
  const float phi = uv.x * kPI * 2.0;
  const float theta = (-uv.y + 0.5) * kPI;
  return vec3(cos(phi) * cos(theta), sin(theta), sin(phi) * cos(theta));
#  elif MODE == _CUBE_NET
  const vec2 kNetSize = vec2(4.0, 3.0);
  const vec2 kInvNetSize = 1.0 / kNetSize;

  const ivec2 coord = ivec2(floor(uv / kInvNetSize));
  const vec2 offset = kInvNetSize * vec2(coord);

  uv = (uv - offset) * kNetSize; // [-1, 1]
  uv = uv * 2.0 - 1.0;           // [0, 1]

  // coord is relative to top-left corner [0, 0] to [kGridSize - 1]
  if (coord == ivec2(3, 1)) {
    return getCubeFace(uv, POSITIVE_X);
  } else if (coord == ivec2(1, 1)) {
    return getCubeFace(uv, NEGATIVE_X);
  } else if (coord == ivec2(2, 0)) {
    return getCubeFace(uv, POSITIVE_Y);
  } else if (coord == ivec2(2, 2)) {
    return getCubeFace(uv, NEGATIVE_Y);
  } else if (coord == ivec2(2, 1)) {
    return getCubeFace(uv, POSITIVE_Z);
  } else if (coord == ivec2(0, 1)) {
    return getCubeFace(uv, NEGATIVE_Z);
  }
  discard;
#  endif
  return vec3(0.0);
}
#else
layout(binding = 0) uniform sampler2D t_0;
#endif

layout(location = 0) out vec4 FragColor;
void main() {
#if CUBEMAP
  FragColor = fs_in.color * texture(t_0, getCubeCoord(fs_in.texCoord));
#else
#  if USE_MONOCHROMATIC_FONT
  FragColor = fs_in.color * texture(t_0, fs_in.texCoord).r;
#  else
  FragColor = fs_in.color * texture(t_0, fs_in.texCoord);
#  endif
#endif
}
