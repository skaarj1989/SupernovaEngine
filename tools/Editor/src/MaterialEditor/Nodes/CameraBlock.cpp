#include "MaterialEditor/Nodes/CameraBlock.hpp"
#include "MaterialEditor/CameraBlockMember.hpp"
#include "NodesInternal.hpp"

ContainerNode createCameraBlock(ShaderGraph &g, VertexDescriptor parent) {
#define CREATE_OUTPUT(Value)                                                   \
  createInternalOutput(g, parent, std::nullopt, CameraBlockMember::Value)

  return {
    .name = "CameraBlock",
    .outputs =
      {
        CREATE_OUTPUT(Projection),
        CREATE_OUTPUT(InversedProjection),
        CREATE_OUTPUT(View),
        CREATE_OUTPUT(InversedView),
        CREATE_OUTPUT(ViewProjection),
        CREATE_OUTPUT(InversedViewProjection),
        CREATE_OUTPUT(Resolution),
        CREATE_OUTPUT(Near),
        CREATE_OUTPUT(Far),
      },
  };
}
