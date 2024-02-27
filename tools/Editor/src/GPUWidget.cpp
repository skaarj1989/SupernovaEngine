#include "GPUWidget.hpp"
#include "imgui.h"
#include "IconsFontAwesome6.h"
#include <fstream>

void showGPUWindow(const char *name, bool *open, const rhi::RenderDevice &rd) {
  ZoneScopedN("GPUWindow");
  if (ImGui::Begin(name, open)) {
    const auto [vendorId, deviceId, deviceName] = rd.getPhysicalDeviceInfo();
    ImGui::Text("VendorID: %u", vendorId);
    ImGui::Text("DeviceID: %u", deviceId);
    ImGui::Text("DeviceName: %s", deviceName.data());

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::Button(ICON_FA_FILE_EXPORT " Dump Memory Stats")) {
      if (auto f = std::ofstream{"MemoryStats.json"}; f.is_open()) {
        f << rd.getMemoryStats();
      }
    }
  }
  ImGui::End();
}
