#pragma once

#include <cstdint>

namespace gfx {

struct ShadowSettings {
  struct CascadedShadowMaps {
    uint32_t numCascades{4};
    uint32_t shadowMapSize{1024};
    float lambda{0.75f};

    template <class Archive> void serialize(Archive &archive) {
      archive(numCascades, shadowMapSize, lambda);
    }
  };
  CascadedShadowMaps cascadedShadowMaps;

  struct SpotLightShadowMaps {
    uint32_t maxNumShadows{4};
    uint32_t shadowMapSize{1024};

    template <class Archive> void serialize(Archive &archive) {
      archive(maxNumShadows, shadowMapSize);
    }
  };
  SpotLightShadowMaps spotLightShadowMaps;

  struct OmniShadowMaps {
    uint32_t maxNumShadows{4};
    uint32_t shadowMapSize{1024};

    template <class Archive> void serialize(Archive &archive) {
      archive(maxNumShadows, shadowMapSize);
    }
  };
  OmniShadowMaps omniShadowMaps;

  template <class Archive> void serialize(Archive &archive) {
    archive(cascadedShadowMaps, spotLightShadowMaps, omniShadowMaps);
  }
};

} // namespace gfx
