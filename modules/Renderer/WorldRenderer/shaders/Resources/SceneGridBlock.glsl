#ifndef _SCENE_GRID_BLOCK_GLSL_
#define _SCENE_GRID_BLOCK_GLSL_

// UploadSceneGrid.cpp

struct SceneGrid { // ArrayStride = 32 bytes
  vec3 minCorner;  // offset = 0 | size = 12
  float _pad;      //         12 |         4
  uvec3 gridSize;  //         16 |        12
  float cellSize;  //         28 |         4
};

layout(set = 2, binding = 9, std140) uniform _SceneGridBlock {
  SceneGrid u_SceneGrid;
};

#endif
