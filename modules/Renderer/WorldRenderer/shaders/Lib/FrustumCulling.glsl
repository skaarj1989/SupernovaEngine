#ifndef _FRUSTUM_CULLING_GLSL_
#define _FRUSTUM_CULLING_GLSL_

#include "Frustum.glsl"
#include "Sphere.glsl"
#include "Cone.glsl"

// NOTE: All planes face inward

bool pointBehindPlane(vec3 p, Plane plane) {
  return distanceToPlane(plane, p) < 0.0;
}

bool sphereBehindPlane(Sphere sphere, Plane plane) {
  return distanceToPlane(plane, sphere.c) < -sphere.r;
}

bool sphereInsideFrustum(Sphere sphere, Frustum frustum, float zNear,
                         float zFar) {
  bool inside = true;
  if (sphere.c.z - sphere.r > zNear || sphere.c.z + sphere.r < zFar) {
    inside = false;
  }
  for (uint i = 0; i < 4 && inside; ++i) {
    inside = !sphereBehindPlane(sphere, frustum.planes[i]);
  }
  return inside;
}

bool coneBehindPlane(Cone cone, Plane plane) {
  // Compute the farthest point on the end of the cone to the positive space of
  // the plane.
  const vec3 m = cross(cross(plane.n, cone.d), cone.d);
  const vec3 Q = cone.T + cone.d * cone.h - m * cone.r;

  // The cone is in the negative halfspace of the plane if both
  // the tip of the cone and the farthest point on the end of the cone to the
  // positive halfspace of the plane are both inside the negative halfspace
  // of the plane.
  return pointBehindPlane(cone.T, plane) && pointBehindPlane(Q, plane);
}

bool coneInsideFrustum(Cone cone, Frustum frustum, float zNear, float zFar) {
  const Plane nearPlane = {vec3(0.0, 0.0, -1.0), zNear};
  const Plane farPlane = {vec3(0.0, 0.0, 1.0), -zFar};

  bool inside = true;
  if (coneBehindPlane(cone, nearPlane) || coneBehindPlane(cone, farPlane)) {
    inside = false;
  }
  for (uint i = 0; i < 4 && inside; ++i) {
    inside = !coneBehindPlane(cone, frustum.planes[i]);
  }
  return inside;
}

#endif
