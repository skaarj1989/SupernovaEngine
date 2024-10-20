#include "RenderSettings.hpp"
#include "renderer/RenderSettings.hpp"

#include "ImGuiHelper.hpp" // ComboEx
#include "imgui_internal.h"

#include "glm/gtc/type_ptr.hpp" // value_ptr

namespace {

void inspect(gfx::OutputMode &outputMode) {
  ImGui::ComboEx("outputMode", outputMode,
                 "Depth\0"
                 "Normal\0"
                 "Emissive\0"
                 "BaseColor\0"
                 "Metallic\0"
                 "Roughness\0"
                 "AmbientOcclusion\0"
                 "SSAO\0"
                 "BrightColor\0"
                 "Reflections\0"
                 "Accum\0"
                 "Reveal\0"
                 "LightHeatmap\0"
                 "HDR\0"
                 "FinalImage\0"
                 "\0");
}

void inspect(gfx::RenderFeatures &features) {
  if (ImGui::BeginCombo("features", nullptr, ImGuiComboFlags_NoPreview)) {
#define FEATURE_CHECKBOX(Name)                                                 \
  ImGui::MenuItemFlags(#Name, features, gfx::RenderFeatures::Name)

    ImGui::PushItemFlag(ImGuiItemFlags_AutoClosePopups, false);
    FEATURE_CHECKBOX(LightCulling);
    FEATURE_CHECKBOX(SoftShadows);
    FEATURE_CHECKBOX(GI);
    FEATURE_CHECKBOX(SSAO);
    FEATURE_CHECKBOX(SSR);
    FEATURE_CHECKBOX(Bloom);
    FEATURE_CHECKBOX(FXAA);
    FEATURE_CHECKBOX(EyeAdaptation);
    FEATURE_CHECKBOX(CustomPostprocess);
    ImGui::PopItemFlag();

    ImGui::EndCombo();
  }
#undef FEATURE_CHECKBOX
}

void inspect(gfx::RenderSettings::GlobalIllumination &settings) {
  ImGui::PushItemWidth(100);
  ImGui::SliderInt("numPropagations", &settings.numPropagations, 1, 12, nullptr,
                   ImGuiSliderFlags_AlwaysClamp);
  ImGui::SliderFloat("intensity", &settings.intensity, 0.0f, 10.0f, "%.3f",
                     ImGuiSliderFlags_AlwaysClamp);
  ImGui::PopItemWidth();
}

void inspectShadowMapSize(uint32_t &size) {
  ImGui::SliderInt("shadowMapSize", std::bit_cast<int32_t *>(&size), 512, 4096,
                   nullptr, ImGuiSliderFlags_AlwaysClamp);
}
void inspect(gfx::ShadowSettings::CascadedShadowMaps &settings) {
  ImGui::PushItemWidth(100);
  ImGui::SliderInt("numCascades",
                   std::bit_cast<int32_t *>(&settings.numCascades), 1, 4,
                   nullptr, ImGuiSliderFlags_AlwaysClamp);
  inspectShadowMapSize(settings.shadowMapSize);
  ImGui::SliderFloat("lambda", &settings.lambda, 0.01f, 1.0f);
  ImGui::PopItemWidth();
}
void inspect(gfx::ShadowSettings::SpotLightShadowMaps &settings) {
  inspectShadowMapSize(settings.shadowMapSize);
}
void inspect(gfx::ShadowSettings::OmniShadowMaps &settings) {
  inspectShadowMapSize(settings.shadowMapSize);
}
void inspect(gfx::ShadowSettings &settings) {
  ImGui::SeparatorText("cascadedShadowMaps");
  ImGui::PushID(&settings.cascadedShadowMaps);
  inspect(settings.cascadedShadowMaps);
  ImGui::PopID();

  ImGui::SeparatorText("spotLightShadowMaps");
  ImGui::PushID(&settings.spotLightShadowMaps);
  inspect(settings.spotLightShadowMaps);
  ImGui::PopID();

  ImGui::SeparatorText("omniShadowMaps");
  ImGui::PushID(&settings.omniShadowMaps);
  inspect(settings.omniShadowMaps);
  ImGui::PopID();
}

void inspect(gfx::SSAOSettings &settings) {
  ImGui::PushItemWidth(100);
  ImGui::SliderFloat("radius##ssao", &settings.radius, 0.1f, 10.0f, "%.3f",
                     ImGuiSliderFlags_AlwaysClamp);
  ImGui::SliderFloat("power##ssao", &settings.power, 0.1f, 10.0f, "%.3f",
                     ImGuiSliderFlags_AlwaysClamp);
  ImGui::SliderFloat("bias##ssao", &settings.bias, 0.0f, 0.1f, "%.3f",
                     ImGuiSliderFlags_AlwaysClamp);
  ImGui::PopItemWidth();
}

void inspect(gfx::RenderSettings::Bloom &settings) {
  ImGui::PushItemWidth(100);
  ImGui::SliderFloat("radius##bloom", &settings.radius, 0.001f, 0.5f, "%.4f",
                     ImGuiSliderFlags_AlwaysClamp);
  ImGui::SliderFloat("strength##bloom", &settings.strength, 0.001f, 0.5f,
                     "%.4f", ImGuiSliderFlags_AlwaysClamp);
  ImGui::PopItemWidth();
}

void inspect(gfx::AdaptiveExposure &settings) {
  ImGui::SliderFloat("minLogLuminance", &settings.minLogLuminance, -100.0f,
                     100.0f);
  ImGui::SliderFloat("maxLogLuminance", &settings.maxLogLuminance, -100.0f,
                     100.0f);
  ImGui::SliderFloat("tau", &settings.tau, 0.001f, 10.0f);
}
void inspect(gfx::Tonemap &tonemap) {
  ImGui::ComboEx("tonemap", tonemap,
                 "Clamp\0"
                 "ACES\0"
                 "Filmic\0"
                 "Reinhard\0"
                 "Uncharted\0"
                 "\0");
}

void inspect(gfx::DebugFlags &flags) {
  if (ImGui::BeginCombo("debugFlags", nullptr, ImGuiComboFlags_NoPreview)) {
#define DEBUG_CHECKBOX(Name)                                                   \
  ImGui::MenuItemFlags(#Name, flags, gfx::DebugFlags::Name)

    ImGui::PushItemFlag(ImGuiItemFlags_AutoClosePopups, false);
    DEBUG_CHECKBOX(WorldBounds);
    DEBUG_CHECKBOX(InfiniteGrid);
    DEBUG_CHECKBOX(Wireframe);
    DEBUG_CHECKBOX(VertexNormal);
    DEBUG_CHECKBOX(CascadeSplits);
    DEBUG_CHECKBOX(LightHeatmap);
    DEBUG_CHECKBOX(VPL);
    DEBUG_CHECKBOX(IrradianceOnly);
    ImGui::PopItemFlag();

    ImGui::EndCombo();
  }
#undef DEBUG_CHECKBOX
}

} // namespace

void showRenderSettings(gfx::RenderSettings &settings) {
  ImGui::SetNextItemWidth(150);
  inspect(settings.outputMode);

  ImGui::Dummy({0, 4});

  ImGui::ColorEdit4("ambientLight", glm::value_ptr(settings.ambientLight),
                    ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float);

  ImGui::SetNextItemWidth(100);
  ImGui::SliderFloat("IBLIntensity", &settings.IBLIntensity, 0.0f, 5.0f, "%.3f",
                     ImGuiSliderFlags_AlwaysClamp);

  const auto hasFeatures = [&settings](gfx::RenderFeatures flags) {
    return (settings.features & flags) == flags;
  };

  ImGui::BeginDisabled(!hasFeatures(gfx::RenderFeatures::GI));
  ImGui::Frame([&settings] {
    if (ImGui::TreeNode("globalIllumination")) {
      inspect(settings.globalIllumination);
      ImGui::TreePop();
    }
  });
  ImGui::EndDisabled();

  ImGui::Frame([&settings] {
    if (ImGui::TreeNode("shadow")) {
      inspect(settings.shadow);
      ImGui::TreePop();
    }
  });

  ImGui::BeginDisabled(!hasFeatures(gfx::RenderFeatures::SSAO));
  ImGui::Frame([&settings] {
    if (ImGui::TreeNode("ssao")) {
      inspect(settings.ssao);
      ImGui::TreePop();
    }
  });
  ImGui::EndDisabled();

  ImGui::BeginDisabled(!hasFeatures(gfx::RenderFeatures::Bloom));
  ImGui::Frame([&settings] {
    if (ImGui::TreeNode("bloom")) {
      inspect(settings.bloom);
      ImGui::TreePop();
    }
  });
  ImGui::EndDisabled();

  ImGui::Frame([&settings, hasAutoExposure =
                             hasFeatures(gfx::RenderFeatures::EyeAdaptation)] {
    ImGui::PushItemWidth(100);

    ImGui::SeparatorText("adaptiveExposure");

    ImGui::BeginDisabled(!hasAutoExposure);
    inspect(settings.adaptiveExposure);
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginDisabled(hasAutoExposure);
    ImGui::SliderFloat("exposure", &settings.exposure, 0.01f, 64.0f, "%.3f",
                       ImGuiSliderFlags_Logarithmic |
                         ImGuiSliderFlags_AlwaysClamp);
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginDisabled(settings.outputMode != gfx::OutputMode::FinalImage);
    inspect(settings.tonemap);
    ImGui::EndDisabled();

    ImGui::PopItemWidth();
  });

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  inspect(settings.features);
  ImGui::SameLine();
  inspect(settings.debugFlags);
}
