#pragma once

#include "glad/vulkan.h"
#include <span>

namespace rhi {

class RenderDevice;
class Buffer;
class Texture;

void upload(RenderDevice &, const Buffer &srcStagingBuffer,
            std::span<const VkBufferImageCopy> copyRegions, Texture &dst,
            bool generateMipmaps = false);

} // namespace rhi
