#ifndef _SPACE_TRAVERSAL_GLSL_
#define _SPACE_TRAVERSAL_GLSL_

#ifndef _CAMERA_BLOCK_GLSL_
#  error "CameraBlock.glsl required"
#endif

/*
Object coords
  * modelMatrix
World coords
  * viewMatrix
Eye coords
  * projectionMatrix
Clip coords
  / .w
NDC
  scale/translate to viewport
Window coords
*/

vec3 worldToView(vec4 v) { return vec3(u_Camera.view * v); }
vec4 worldToClip(vec4 v) { return u_Camera.viewProjection * v; }
vec3 worldToNDC(vec4 v) {
  const vec4 P = worldToClip(v);
  return P.xyz / P.w;
}

vec3 viewToWorld(vec4 v) { return vec3(u_Camera.inversedView * v); }
vec4 viewToClip(vec4 v) { return u_Camera.projection * v; }
vec3 viewToNDC(vec4 v) {
  const vec4 P = viewToClip(v);
  return P.xyz / P.w;
}

vec3 NDCToView(vec4 v) {
  const vec4 P = u_Camera.inversedProjection * v;
  return P.xyz / P.w;
}
vec3 NDCToWorld(vec4 v) {
  const vec4 P = u_Camera.inversedViewProjection * v;
  return P.xyz / P.w;
}

vec2 screenToNDC(vec2 screen) {
  return screen * getScreenTexelSize() * 2.0 - 1.0;
}

vec3 screenToView(vec2 screen, float z, float w) {
  const vec4 ndc = vec4(screenToNDC(screen), z, w);
  return NDCToView(ndc);
}

#endif
