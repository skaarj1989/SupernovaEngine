#pragma once

#include "rhi/ImageAspect.hpp"
#include "rhi/CubeFace.hpp"
#include "PipelineStage.hpp"
#include <cstdint>
#include <optional>

namespace gfx {

enum class ClearValue {
  Zero,
  One,

  OpaqueBlack,
  OpaqueWhite,
  TransparentBlack,
  TransparentWhite,
};

struct Attachment {
  uint32_t index{0};
  rhi::ImageAspect imageAspect;
  std::optional<uint32_t> layer;
  std::optional<rhi::CubeFace> face;
  std::optional<ClearValue> clearValue;

  [[nodiscard]] operator uint32_t() const;
};
[[nodiscard]] Attachment decodeAttachment(uint32_t bits);

[[nodiscard]] bool holdsAttachment(uint32_t bits);

struct Location {
  uint32_t set{0};
  uint32_t binding{0};

  [[nodiscard]] operator uint32_t() const;
};
[[nodiscard]] Location decodeLocation(uint32_t bits);

struct BindingInfo {
  Location location;
  PipelineStage pipelineStage;

  [[nodiscard]] operator uint32_t() const;
};
[[nodiscard]] BindingInfo decodeBindingInfo(uint32_t bits);

struct TextureRead {
  BindingInfo binding;

  enum class Type { CombinedImageSampler, SampledImage, StorageImage };
  Type type;
  rhi::ImageAspect imageAspect;

  [[nodiscard]] operator uint32_t() const;
};
[[nodiscard]] TextureRead decodeTextureRead(uint32_t bits);

struct ImageWrite {
  BindingInfo binding;
  rhi::ImageAspect imageAspect;

  [[nodiscard]] operator uint32_t() const;
};
[[nodiscard]] ImageWrite decodeImageWrite(uint32_t bits);

} // namespace gfx
