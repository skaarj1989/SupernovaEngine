#pragma once

namespace rhi {
class RenderDevice;
}

void showGPUWindow(const char *name, bool *open, const rhi::RenderDevice &);
