#include "Inspectors/AnimationInspector.hpp"
#include "imgui.h"

void print(const ozz::animation::Animation &animation) {
  ImGui::BulletText("Duration: %.2f", animation.duration());
  ImGui::BulletText("Num tracks: %d", animation.num_tracks());
}
