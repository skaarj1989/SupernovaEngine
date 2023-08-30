#include "renderer/SkyLight.hpp"

namespace gfx {

SkyLight::operator bool() const {
  return environment && *environment && diffuse && specular;
}

} // namespace gfx
