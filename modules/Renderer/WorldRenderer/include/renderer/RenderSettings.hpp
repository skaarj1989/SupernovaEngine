#pragma once

#include "OutputMode.hpp"
#include "RenderFeatures.hpp"
#include "ShadowSettings.hpp"
#include "SSAOSettings.hpp"
#include "AdaptiveExposure.hpp"
#include "Tonemap.hpp"
#include "DebugFlags.hpp"

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

namespace gfx {

struct RenderSettings {
  OutputMode outputMode{OutputMode::FinalImage};
  RenderFeatures features{RenderFeatures::Default};

  glm::vec4 ambientLight{glm::vec3{0.0f}, 0.1f};
  float IBLIntensity{1.0f};
  struct GlobalIllumination {
    int32_t numPropagations{6};
    float intensity{1.0f};

    template <class Archive> void serialize(Archive &archive) {
      archive(numPropagations, intensity);
    }
  };
  GlobalIllumination globalIllumination;
  ShadowSettings shadow;

  SSAOSettings ssao;

  struct Bloom {
    float radius{0.005f};
    float strength{0.04f};

    template <class Archive> void serialize(Archive &archive) {
      archive(radius, strength);
    }
  };
  Bloom bloom;
  float exposure{1.0f};
  AdaptiveExposure adaptiveExposure;
  Tonemap tonemap{Tonemap::ACES};

  DebugFlags debugFlags{DebugFlags::None};

  template <class Archive> void serialize(Archive &archive) {
    archive(outputMode, features, ambientLight, IBLIntensity,
            globalIllumination, shadow, ssao, bloom, exposure, adaptiveExposure,
            tonemap, debugFlags);
  }
};

} // namespace gfx
