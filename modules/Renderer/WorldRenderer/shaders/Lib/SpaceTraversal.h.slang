#pragma once

// Requires: CameraBlock.h.slang

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

float3 worldToView(float4 v) { return mul(u_Camera.view, v).xyz; }

float4 worldToClip(float4 v) { return mul(u_Camera.viewProjection, v); }

float3 worldToNDC(float4 v) {
  const float4 P = worldToClip(v);
  return P.xyz / P.w;
}

float3 viewToWorld(float4 v) { return mul(u_Camera.inversedView, v).xyz; }

float4 viewToClip(float4 v) { return mul(u_Camera.projection, v); }

float3 viewToNDC(float4 v) {
  const float4 P = viewToClip(v);
  return P.xyz / P.w;
}

float3 NDCToView(float4 v) {
  const float4 P = mul(u_Camera.inversedProjection, v);
  return P.xyz / P.w;
}

float3 NDCToWorld(float4 v) {
  const float4 P = mul(u_Camera.inversedViewProjection, v);
  return P.xyz / P.w;
}

float2 screenToNDC(float2 screen) {
  return screen * u_Camera.getScreenTexelSize() * 2.0 - 1.0;
}

float3 screenToView(float2 screen, float z, float w) {
  const float4 ndc = float4(screenToNDC(screen), z, w);
  return NDCToView(ndc);
}
