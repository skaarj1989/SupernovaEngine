#include "SkeletonInspector.hpp"
#include "ozz/animation/runtime/skeleton.h"
#include "imgui.h"
#include <algorithm>
#include <format>

void print(const ozz::animation::Skeleton &skeleton) {
  const auto &jointNames = skeleton.joint_names();
  if (ImGui::TreeNode(std::format("Joints ({})", jointNames.size()).c_str())) {
    std::ranges::for_each(jointNames, ImGui::Text);
    ImGui::TreePop();
  }
}
