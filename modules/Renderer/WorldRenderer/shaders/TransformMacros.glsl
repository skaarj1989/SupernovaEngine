#ifndef _TRANSFORM_MACROS_GLSL_
#define _TRANSFORM_MACROS_GLSL_

// -- Forward:

#define LOCAL_TO_WORLD_SPACE modelMatrix
#define LOCAL_TO_VIEW_SPACE (u_Camera.view * modelMatrix)
#define LOCAL_TO_CLIP_SPACE (u_Camera.viewProjection * modelMatrix)

#define WORLD_TO_VIEW_SPACE u_Camera.view
#define WORLD_TO_CLIP_SPACE u_Camera.viewProjection

#define VIEW_TO_CLIP_SPACE u_Camera.projection

// -- Backward:

#define CLIP_TO_VIEW_SPACE u_Camera.inversedProjection
#define CLIP_TO_WORLD_SPACE u_Camera.inversedViewProjection
#define CLIP_TO_LOCAL_SPACE                                                    \
  (u_Camera.inversedViewProjection * inverse(modelMatrix))

#define VIEW_TO_WORLD_SPACE u_Camera.inversedView
#define VIEW_TO_LOCAL_SPACE (u_Camera.inversedView * inverse(modelMatrix))

#define WORLD_TO_LOCAL_SPACE inverse(modelMatrix)

#endif
