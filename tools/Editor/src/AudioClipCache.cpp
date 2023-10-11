#include "AudioClipCache.hpp"
#include "ImGuiDragAndDrop.hpp"
#include "CacheInspector.hpp"

namespace {

[[nodiscard]] auto toString(const audio::NumChannels numChannels) {
  switch (numChannels) {
    using enum audio::NumChannels;

  case Invalid:
    return "Invalid";

  case Mono:
    return "Mono";
  case Stereo:
    return "Stereo";

  case Surround1D:
    return "Surround 1D";
  case QuadSurround:
    return "QuadSurround";
  case Surround:
    return "Surround";

  case Surround5_1:
    return "Surround 5.1";
  case Surround6_1:
    return "Surround 6.1";
  case Surround7_1:
    return "Surround 7.1";
  }
  assert(false);
  return "Undefined";
}

void print(const audio::ClipInfo &info) {
  ImGui::BulletText(toString(info.numChannels));
  ImGui::BulletText("bitsPerSample: %d", info.bitsPerSample);
  ImGui::BulletText("sampleRate: %u Hz", info.sampleRate);
  ImGui::BulletText("numSamples: %zu", info.numSamples);
  ImGui::BulletText("duration: %.2f sec", info.duration().count());
}

} // namespace

void show(const char *name, bool *open, AudioClipCache &cache) {
  if (ImGui::Begin(name, open, ImGuiWindowFlags_MenuBar)) {
    defaultMenuBar(cache);
    view(
      cache,
      [](auto id) {
        onDragSource(kImGuiPayloadTypeAudioClip, id,
                     [] { ImGui::Text("AudioClip inside ..."); });
      },
      [](const auto &r) { print(r.getInfo()); }, std::nullopt);
  }
  ImGui::End();
}
