#include "Inspectors/ColliderInspector.hpp"
#include "Inspectors/JoltInspector.hpp"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Collision/Shape/CapsuleShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/MeshShape.h"
#include "Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h"
#include "Jolt/Physics/Collision/Shape/ScaledShape.h"
#include "imgui.h"

namespace {

void print(const char *label, float s) { ImGui::Text("%s = %.2f", label, s); }
void print(const char *label, uint32_t s) { ImGui::Text("%s = %u", label, s); }

void print(const JPH::BoxShape *shape) {
  ImGui::Text("Shape: Box");
  ::print("Half extents", shape->GetHalfExtent());
}
void print(const JPH::CapsuleShape *shape) {
  ImGui::Text("Shape: Capsule");
  print("Radius", shape->GetRadius());
  print("Half height", shape->GetHalfHeightOfCylinder());
}
void print(const JPH::SphereShape *shape) {
  ImGui::Text("Shape: Sphere");
  print("Radius", shape->GetRadius());
}
void print(const JPH::MeshShape *shape) {
  ImGui::Text("Shape: Mesh");
  print("Num triangles", shape->GetStats().mNumTriangles);
}

} // namespace

void print(const JPH::Shape *shape) {
  assert(shape);

  switch (shape->GetSubType()) {
    using enum JPH::EShapeSubType;

  case Box:
    print(static_cast<const JPH::BoxShape *>(shape));
    break;
  case Capsule:
    print(static_cast<const JPH::CapsuleShape *>(shape));
    break;
  case Sphere:
    print(static_cast<const JPH::SphereShape *>(shape));
    break;
  case Mesh: {
    print(static_cast<const JPH::MeshShape *>(shape));
  } break;

  case RotatedTranslated: {
    if (ImGui::TreeNode("RotatedTranslatedShape")) {
      auto rotatedTranslatedShape =
        static_cast<const JPH::RotatedTranslatedShape *>(shape);
      print("Offset", rotatedTranslatedShape->GetPosition());
      print("Orientation", rotatedTranslatedShape->GetRotation());
      ImGui::SeparatorText("Inner");
      print(rotatedTranslatedShape->GetInnerShape());

      ImGui::TreePop();
    }
  } break;
  case Scaled: {
    if (ImGui::TreeNode("ScaledShape")) {
      auto scaledShape = static_cast<const JPH::ScaledShape *>(shape);
      print("Scale", scaledShape->GetScale());
      ImGui::SeparatorText("Inner");
      print(scaledShape->GetInnerShape());

      ImGui::TreePop();
    }
  } break;
  }
}
