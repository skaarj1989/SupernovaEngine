#pragma once

#include "animation/SkeletonManager.hpp"
#include "animation/AnimationManager.hpp"
#include "physics/ColliderManager.hpp"
#include "renderer/MeshManager.hpp"
#include "AudioClipManager.hpp"
#include "ScriptManager.hpp"

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
