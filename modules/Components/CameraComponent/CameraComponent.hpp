#pragma once

#include "renderer/WorldRenderer.hpp"

class CameraComponent {
  static constexpr auto in_place_delete = true;

public:
  CameraComponent() = default;
  explicit CameraComponent(rhi::Extent2D _extent) : extent{_extent} {
    camera.setAspectRatio(extent.getAspectRatio());
  }
  CameraComponent(const CameraComponent &other)
      : extent{other.extent}, camera{other.camera},
        renderSettings{other.renderSettings}, skyLight{other.skyLight},
        postProcessEffects{other.postProcessEffects} {}
  CameraComponent(CameraComponent &&) noexcept = default;
  ~CameraComponent() = default;

  CameraComponent &operator=(const CameraComponent &) = delete;
  CameraComponent &operator=(CameraComponent &&) noexcept = default;

  template <class Archive> void serialize(Archive &archive) {
    archive(extent, camera, renderSettings, skyLight, postProcessEffects);
  }

  rhi::Extent2D extent{};
  std::shared_ptr<rhi::Texture> target;

  gfx::PerspectiveCamera camera;
  gfx::RenderSettings renderSettings{};
  gfx::SkyLight skyLight;
  std::vector<gfx::MaterialInstance> postProcessEffects;
  DebugDraw debugDraw;
};

static_assert(std::is_copy_constructible_v<CameraComponent>);
