#pragma once

namespace gfx {

struct SSAOSettings {
  float radius{0.5f};
  float power{1.0f};
  float bias{0.025f};

  template <class Archive> void serialize(Archive &archive) {
    archive(radius, power, bias);
  }
};

} // namespace gfx
