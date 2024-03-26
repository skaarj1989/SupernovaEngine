#pragma once

#include "glm/ext/vector_float2.hpp"

namespace rhi {
class Texture;
}

void overlay(rhi::Texture *, const glm::vec2 size);
void preview(rhi::Texture *, const glm::vec2 size);
