#include "Inspectors/JoltInspector.hpp"
#include "imgui.h"

void print(const char *label, const JPH::Vec3 &v) {
  ImGui::Text("%s = {\n\t.x = %.2f,\n\t.y = %.2f,\n\t.z = %.2f\n}", label,
              v.GetX(), v.GetY(), v.GetZ());
}
void print(const char *label, const JPH::Quat &q) {
  ImGui::Text(
    "%s = {\n\t.w = %.2f,\n\t.x = %.2f,\n\t.y = %.2f,\n\t.z = %.2f\n}", label,
    q.GetW(), q.GetX(), q.GetY(), q.GetZ());
}