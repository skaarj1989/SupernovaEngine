#include "MaterialEditor/Nodes/FrameBlock.hpp"
#include "MaterialEditor/FrameBlockMember.hpp"
#include "NodesInternal.hpp"

ContainerNode createFrameBlock(ShaderGraph &g, VertexDescriptor parent) {
#define CREATE_OUTPUT(Value)                                                   \
  createInternalOutput(g, parent, std::nullopt, FrameBlockMember::Value)

  return {
    .name = "FrameBlock",
    .outputs =
      {
        CREATE_OUTPUT(Time),
        CREATE_OUTPUT(DeltaTime),
      },
  };
}
