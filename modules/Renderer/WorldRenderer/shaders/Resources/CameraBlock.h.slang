#pragma once

struct Camera {                    // ArrayStride = 400 bytes
  float4x4 projection;             // offset = 0 | size = 64
  float4x4 inversedProjection;     //         64 |        64
  float4x4 view;                   //        128 |        64
  float4x4 inversedView;           //        192 |        64
  float4x4 viewProjection;         //        256 |        64
  float4x4 inversedViewProjection; //        320 |        64
  uint2 resolution;                //        384 |         8
  float near;                      //        392 |         4
  float far;                       //        396 |         4

  float3 getCameraPosition() { return inversedView[3].xyz; }
  float2 getScreenTexelSize() { return 1.0 / float2(resolution); }
  float getAspectRatio() { return float(resolution.x) / float(resolution.y); }
};

[[vk::binding(0, 1)]]
ConstantBuffer<Camera> u_Camera : register(b0);
