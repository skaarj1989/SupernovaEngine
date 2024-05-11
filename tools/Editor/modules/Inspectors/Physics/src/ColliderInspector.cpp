#include "ColliderInspector.hpp"

#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "Jolt/Jolt.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/ConvexHullShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/StaticCompoundShape.h"
#include "Jolt/Physics/Collision/Shape/ScaledShape.h"
#include "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h"
#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

#include "imgui.h"

namespace {

void print(const JPH::SphereShape *shape) {
  ImGui::Text("Sphere{ .radius = %.2f }", shape->GetRadius());
}
void print(const JPH::BoxShape *shape) {
  const auto &v = shape->GetHalfExtent();
  ImGui::Text("Box{ .halfExtent = {%.2f, %.2f, %.2f} }", v.GetX(), v.GetY(),
              v.GetZ());
}
void print(const JPH::CapsuleShape *shape) {
  ImGui::Text("Capsule{ .radius = %.2f, .halfHeight = %.2f }",
              shape->GetRadius(), shape->GetHalfHeightOfCylinder());
}
void print(const JPH::ConvexHullShape *shape) {
  ImGui::Text("ConvexHull{ .numPoints = %u }", shape->GetNumPoints());
}

void print(const JPH::MeshShape *shape) {
  ImGui::Text("Mesh{ .numTriangles = %u }", shape->GetStats().mNumTriangles);
}

void print(const JPH::StaticCompoundShape *shape) {
  ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
  if (ImGui::TreeNode("Compound")) {
    for (auto i = 0u; i < shape->GetNumSubShapes(); ++i) {
      ::print(shape->GetSubShape(i).mShape.GetPtr());
    }
    ImGui::TreePop();
  }
}

void print(const JPH::ScaledShape *shape) {
  ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
  if (ImGui::TreeNode("Scaled")) {
    const auto &s = shape->GetScale();
    ImGui::Text("{ .scale = {%.2f, %.2f, %.2f} }", s.GetX(), s.GetY(),
                s.GetZ());
    ::print(shape->GetInnerShape());

    ImGui::TreePop();
  }
}
void print(const JPH::RotatedTranslatedShape *shape) {
  ImGui::SetNextItemOpen(true, ImGuiCond_FirstUseEver);
  if (ImGui::TreeNode("RotatedTranslated")) {
    const auto &p = shape->GetPosition();
    const auto &q = shape->GetRotation();
    ImGui::Text("{\n  .offset = {%.2f, %.2f, %.2f},\n  .orientation = "
                "{%.2f, %.2f,%.2f, .w=%.2f}\n}",
                p.GetX(), p.GetY(), p.GetZ(), q.GetX(), q.GetY(), q.GetZ(),
                q.GetW());
    ::print(shape->GetInnerShape());

    ImGui::TreePop();
  }
}

} // namespace

void print(const JPH::Shape *shape) {
  assert(shape);

  switch (shape->GetSubType()) {
#define CASE(T)                                                                \
  case JPH::EShapeSubType::T:                                                  \
    return print(static_cast<const JPH::T##Shape *>(shape))

    CASE(Sphere);
    CASE(Box);
    CASE(Capsule);
    CASE(ConvexHull);

    CASE(StaticCompound);

    CASE(Scaled);
    CASE(RotatedTranslated);

    CASE(Mesh);
#undef CASE

  default:
    ImGui::TextUnformatted("Unknown");
  }
}
