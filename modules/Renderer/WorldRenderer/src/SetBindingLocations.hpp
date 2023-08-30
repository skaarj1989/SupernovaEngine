#pragma once

#include <utility>

namespace gfx {

const auto kFrameBlockLocation = {0u, 0u};
const auto kPropertyBufferLocation = {0, 1};
const auto kSkinBufferLocation = {0, 2};

const auto kCameraBlockLocation = {1, 0};
const auto kTransformBufferLocation = {1, 1};
const auto kInstanceBufferLocation = {1, 2};
const auto kLightsLocation = {1, 3};
const auto kShadowBlockLocation = {1, 4};

const auto kGBuffer0Location = {1, 5};
const auto kGBuffer1Location = {1, 6};
const auto kGBuffer2Location = {1, 7};
const auto kGBuffer3Location = {1, 8};
const auto kGBuffer4Location = {1, 9};
const auto kGBuffer5Location = {1, 10};

const auto kSceneDepthLocation = {1, 5};
const auto kSceneColorLocation = {1, 11};

const auto kBRDFLocation = {2, 0};
const auto kSkyLightDiffuseLocation = {2, 1};
const auto kSkyLightSpecularLocation = {2, 2};

const auto kCascadedShadowMapsLocation = {2, 3};
const auto kSpotLightShadowMapsLocation = {2, 4};
const auto kOmniShadowMapsLocation = {2, 5};

const auto kSSAOLocation = {2, 6};

const auto kLightGridLocation = {2, 7};
const auto kLightIndicesLocation = {2, 8};

const auto kSceneGridBlockLocation = {2, 9};
const auto kLPV0Location = {2, 10};
const auto kLPV1Location = {2, 11};
const auto kLPV2Location = {2, 12};

} // namespace gfx
