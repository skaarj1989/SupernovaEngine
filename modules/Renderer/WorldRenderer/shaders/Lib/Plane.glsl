#ifndef _PLANE_GLSL_
#define _PLANE_GLSL_

struct Plane {
  vec3 n;  // Normal
  float d; // Distance to origin
};

// Compute a plane from 3 noncollinear points that form a triangle.
// This equation assumes a right-handed (counter-clockwise winding order)
// coordinate system to determine the direction of the plane normal.
Plane computePlane(vec3 p0, vec3 p1, vec3 p2) {
  Plane plane;
  const vec3 v0 = p1 - p0;
  const vec3 v1 = p2 - p0;
  plane.n = normalize(cross(v0, v1));
  plane.d = dot(plane.n, p0);
  return plane;
}

float distanceToPlane(Plane plane, vec3 p) { return dot(plane.n, p) + plane.d; }

#endif
