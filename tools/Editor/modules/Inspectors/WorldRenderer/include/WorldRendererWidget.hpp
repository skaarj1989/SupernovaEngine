#pragma once

namespace gfx {
class WorldRenderer;
}

void showWorldRendererWindow(const char *name, bool *open,
                             gfx::WorldRenderer &);
