#pragma once

template <class T> void inspect(T &component) {
  const auto &settings = component.getSettings();
  if (auto s = settings.pitch;
      ImGui::DragFloat("pitch", &s, 0.1f, 0.0f, 1000.0f, "%.3f",
                       ImGuiSliderFlags_AlwaysClamp)) {
    component.setPitch(s);
  }
  if (auto s = settings.gain; ImGui::SliderFloat(
        "gain", &s, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
    component.setGain(s);
  }
  if (auto s = settings.maxDistance;
      ImGui::DragFloat("maxDistance", &s, 0.1f, 0.0f, 10'000.0f, "%.3f",
                       ImGuiSliderFlags_AlwaysClamp)) {
    component.setMaxDistance(s);
  }
  if (auto s = settings.rollOffFactor;
      ImGui::SliderFloat("roffOffFactor", &s, 0.0f, 1.0f, "%.3f",
                         ImGuiSliderFlags_AlwaysClamp)) {
    component.setRollOffFactor(s);
  }
  if (auto s = settings.referenceDistance;
      ImGui::DragFloat("referenceDistance", &s, 0.1f, 0.0f, 10'000.0f, "%.3f",
                       ImGuiSliderFlags_AlwaysClamp)) {
    component.setReferenceDistance(s);
  }
  if (auto s = settings.minGain; ImGui::SliderFloat(
        "minGain", &s, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
    component.setMinGain(s);
  }
  if (auto s = settings.maxGain; ImGui::SliderFloat(
        "maxGain", &s, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp)) {
    component.setMaxGain(s);
  }

  if (auto b = settings.directional; ImGui::Checkbox("directional", &b)) {
    component.setDirectional(b);
  }
  if (auto b = settings.loop; ImGui::Checkbox("looping", &b)) {
    component.setLooping(b);
  }
}
