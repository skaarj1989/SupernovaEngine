#pragma once

struct Plane {
  float3 n; // Normal
  float d;  // Distance to origin
};

// Compute a plane from 3 noncollinear points that form a triangle.
// This equation assumes a right-handed (counter-clockwise winding order)
// coordinate system to determine the direction of the plane normal.
Plane computePlane(float3 p0, float3 p1, float3 p2) {
  Plane plane;
  const float3 v0 = p1 - p0;
  const float3 v1 = p2 - p0;
  plane.n = normalize(cross(v0, v1));
  plane.d = dot(plane.n, p0);
  return plane;
}

float distanceToPlane(Plane plane, float3 p) { return dot(plane.n, p) + plane.d; }
