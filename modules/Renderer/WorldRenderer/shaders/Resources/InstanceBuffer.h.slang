#pragma once

struct PushConstant {
  uint instanceOffset;
};

[[vk::push_constant]]
ConstantBuffer<PushConstant> u_PC;

[[vk::binding(2, 1)]]
StructuredBuffer<Instance> g_Instances : register(t0);
