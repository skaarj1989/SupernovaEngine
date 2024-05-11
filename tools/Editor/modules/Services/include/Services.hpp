#pragma once

#include "entt/locator/locator.hpp"

namespace rhi {
class RenderDevice;
}
namespace gfx {
class MeshManager;
class MaterialManager;
class TextureManager;
} // namespace gfx

class SkeletonManager;
class AnimationManager;

class ColliderManager;

namespace audio {
class Device;
}
class AudioClipManager;

class ScriptManager;

struct Services {
  Services() = delete;

  static void init(rhi::RenderDevice &, audio::Device &);
  static void reset();

  struct Resources {
    Resources() = delete;

    static void clear();

    using Skeletons = entt::locator<SkeletonManager>;
    using Animations = entt::locator<AnimationManager>;

    using Colliders = entt::locator<ColliderManager>;

    using Meshes = entt::locator<gfx::MeshManager>;
    using Materials = entt::locator<gfx::MaterialManager>;
    using Textures = entt::locator<gfx::TextureManager>;

    using AudioClips = entt::locator<AudioClipManager>;

    using Scripts = entt::locator<ScriptManager>;
  };
};
