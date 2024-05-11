#include "SceneEditor.hpp"
#include "IconsFontAwesome6.h"

void SceneEditor::_onInspect(entt::handle,
                             PlaybackController &playbackController) const {
  if (auto b = playbackController.isLooped(); ImGui::Checkbox("Loop", &b)) {
    playbackController.setLoop(b);
  }
  ImGui::SameLine();

  const auto playing = playbackController.isPlaying();

  ImGui::BeginDisabled(playing);
  if (ImGui::Button(ICON_FA_PLAY)) playbackController.play();
  ImGui::EndDisabled();

  ImGui::SameLine();

  ImGui::BeginDisabled(!playing);
  if (ImGui::Button(ICON_FA_PAUSE)) playbackController.pause();
  ImGui::EndDisabled();

  ImGui::SameLine();
  if (ImGui::Button(ICON_FA_STOP)) playbackController.stop();

  ImGui::SetNextItemWidth(100);
  if (auto f = playbackController.getPlaybackSpeed();
      ImGui::SliderFloat("Speed", &f, 0.001f, 10.0f)) {
    playbackController.setSpeed(f);
  }
}
