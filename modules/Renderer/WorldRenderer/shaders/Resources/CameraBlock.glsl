#ifndef _CAMERA_BLOCK_GLSL_
#define _CAMERA_BLOCK_GLSL_

// UploadCameraBlock.cpp

struct Camera {                // ArrayStride = 400 bytes
  mat4 projection;             // offset = 0 | size = 64
  mat4 inversedProjection;     //         64 |        64
  mat4 view;                   //        128 |        64
  mat4 inversedView;           //        192 |        64
  mat4 viewProjection;         //        256 |        64
  mat4 inversedViewProjection; //        320 |        64
  uvec2 resolution;            //        384 |         8
  float near;                  //        392 |         4
  float far;                   //        396 |         4
};

layout(set = 1, binding = 0, std140) uniform _CameraBlock { Camera u_Camera; };

vec3 getCameraPosition() { return u_Camera.inversedView[3].xyz; }

uvec2 getResolution() { return u_Camera.resolution; }
vec2 getScreenTexelSize() { return 1.0 / vec2(u_Camera.resolution); }

float getAspectRatio() {
  return float(u_Camera.resolution.x) / float(u_Camera.resolution.y);
}

#endif
