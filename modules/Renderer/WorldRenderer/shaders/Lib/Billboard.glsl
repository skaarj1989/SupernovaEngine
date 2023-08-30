#ifndef _BILLBOARD_GLSL_
#define _BILLBOARD_GLSL_

vec4 billboard(mat4 modelMatrix, vec4 P, bool spherical) {
  mat4 modelViewMatrix = u_Camera.view * modelMatrix;

  modelViewMatrix[0].xyz = vec3(1, 0, 0);
  if (spherical) modelViewMatrix[1].xyz = vec3(0, 1, 0);
  modelViewMatrix[2].xyz = vec3(0, 0, 1);

  return inverse(u_Camera.viewProjection * modelMatrix) *
         (u_Camera.projection * modelViewMatrix * P);
}

#endif
