#pragma once

// renderer/Material.hpp

#define SHADING_MODEL_UNLIT 0
#define SHADING_MODEL_LIT 1

#define BLEND_MODE_OPAQUE 0
#define BLEND_MODE_MASKED 1
#define BLEND_MODE_TRANSPARENT 2

static const uint MaterialFlag_CastShadow = 1 << 0;
static const uint MaterialFlag_ReceiveShadow = 1 << 1;
