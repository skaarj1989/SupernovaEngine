#include "rhi/Extent2D.hpp"
#include "glm/ext/scalar_constants.inl" // epsilon

namespace rhi {

Extent2D::operator bool() const { return width > 0 && height > 0; }

Extent2D::operator VkExtent2D() const { return {width, height}; }
Extent2D::operator glm::uvec2() const { return {width, height}; }

float Extent2D::getAspectRatio() const {
  return height > 0 ? float(width) / float(height) : glm::epsilon<float>();
}

} // namespace rhi
