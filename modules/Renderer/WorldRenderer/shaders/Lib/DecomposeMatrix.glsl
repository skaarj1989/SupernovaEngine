#ifndef _MATRIX_GLSL_
#define _MATRIX_GLSL_

vec3 decomposePosition(mat4 m) { return m[3].xyz; }

// Compute rotation-independant scaling of the model matrix.
vec3 decomposeScale(mat4 m) {
  // clang-format off
  return vec3(
    length(vec3(m[0])),
    length(vec3(m[1])),
    length(vec3(m[2]))
  );
  // clang-format on
}

#endif
