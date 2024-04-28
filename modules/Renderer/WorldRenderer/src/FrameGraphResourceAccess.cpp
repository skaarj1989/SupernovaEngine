#include "FrameGraphResourceAccess.hpp"
#include "glm/integer.hpp" // bitfield{Insert/Extract}
#include <utility>

namespace gfx {

constexpr auto kReservedBitsOffset = 0;
constexpr auto kReservedBits = 1;

constexpr auto kAttachmentMarker = 1u;
constexpr auto kNonAttachmentMarker = 0u;

//
// Attachment  (23 bits):
//
// |  1 bit   | 3 bits | 2 bits | 11 bits |  3 bits  |   3 bits   |
// |   [0]    | [1..3] | [4..5] | [6..16] | [17..19] |  [20..22]  |
// | reserved |  index | aspect |  layer  |   face   | clearValue |

constexpr auto kAttachmentIndexBits = 3;
constexpr auto kImageAspectBits = 2;
constexpr auto kLayerBits = 11;
constexpr auto kFaceBits = 3;
constexpr auto kClearValueBits = 3;

constexpr auto kAttachmentIndexOffset = kReservedBits;
constexpr auto kImageAspectOffset =
  kAttachmentIndexOffset + kAttachmentIndexBits;
constexpr auto kLayerOffset = kImageAspectOffset + kImageAspectBits;
constexpr auto kFaceOffset = kLayerOffset + kLayerBits;
constexpr auto kClearOffset = kFaceOffset + kFaceBits;

[[nodiscard]] auto encode(const Attachment &v) {
  uint32_t encoded{0};
  encoded = glm::bitfieldInsert(encoded, kAttachmentMarker, kReservedBitsOffset,
                                kReservedBits);
  assert(v.imageAspect != rhi::ImageAspect::None);
  encoded = glm::bitfieldInsert(encoded, static_cast<uint32_t>(v.imageAspect),
                                kImageAspectOffset, kImageAspectBits);
  encoded = glm::bitfieldInsert(encoded, v.index, kAttachmentIndexOffset,
                                kAttachmentIndexBits);
  encoded = glm::bitfieldInsert(encoded, v.layer ? *v.layer + 1u : 0u,
                                kLayerOffset, kLayerBits);
  encoded =
    glm::bitfieldInsert(encoded, v.face ? std::to_underlying(*v.face) + 1u : 0u,
                        kFaceOffset, kFaceBits);
  encoded = glm::bitfieldInsert(
    encoded, v.clearValue ? static_cast<uint32_t>(*v.clearValue) + 1 : 0,
    kClearOffset, kClearValueBits);
  return encoded;
}
Attachment decodeAttachment(uint32_t v) {
  Attachment out;
  out.index =
    glm::bitfieldExtract(v, kAttachmentIndexOffset, kAttachmentIndexBits);

  out.imageAspect = static_cast<rhi::ImageAspect>(
    glm::bitfieldExtract(v, kImageAspectOffset, kImageAspectBits));
  assert(out.imageAspect != rhi::ImageAspect::None);

  // nullopt is encoded as '0'
  if (const auto temp = glm::bitfieldExtract(v, kLayerOffset, kLayerBits);
      temp != 0) {
    out.layer = temp - 1;
  }
  if (const auto temp = glm::bitfieldExtract(v, kFaceOffset, kFaceBits);
      temp != 0) {
    out.face = static_cast<rhi::CubeFace>(temp - 1);
  }
  if (const auto temp = glm::bitfieldExtract(v, kClearOffset, kClearValueBits);
      temp != 0) {
    out.clearValue = static_cast<ClearValue>(temp - 1);
  }
  return out;
}

//
// Location (7 bits):
//
// | 2 bits | 5 bits  |
// | [0..1] | [2..6]  |
// |  set   | binding |

constexpr auto kLocationBits = 7;

constexpr auto kSetIndexBits = 2;
constexpr auto kBindingIndexBits = 5;

constexpr auto kSetIndexOffset = 0;
constexpr auto kBindingIndexOffset = kSetIndexOffset + kSetIndexBits;

[[nodiscard]] auto encode(const Location &v) {
  uint32_t bits{0};
  bits = glm::bitfieldInsert(bits, v.set, kSetIndexOffset, kSetIndexBits);
  bits = glm::bitfieldInsert(bits, v.binding, kBindingIndexOffset,
                             kBindingIndexBits);
  return bits;
}
Location decodeLocation(uint32_t bits) {
  return {
    .set = glm::bitfieldExtract(bits, kSetIndexOffset, kSetIndexBits),
    .binding =
      glm::bitfieldExtract(bits, kBindingIndexOffset, kBindingIndexBits),
  };
}

bool holdsAttachment(uint32_t bits) {
  return glm::bitfieldExtract(bits, kReservedBitsOffset, kReservedBits) ==
         kAttachmentMarker;
}

//
// BindingInfo (13 bits):
//
// |  1 bit   |  7 bits  |    5 bits     |
// |   [0]    |  [1..7]  |   [8..12]     |
// | reserved | location | pipelineStage |

constexpr auto kBindingInfoBits = 13;

constexpr auto kPipelineStageBits = 5;

constexpr auto kLocationOffset = kReservedBits;
constexpr auto kPipelineStageOffset = kLocationOffset + kLocationBits;

[[nodiscard]] auto encode(const BindingInfo &v) {
  uint32_t bits{0};
  bits = glm::bitfieldInsert(bits, kNonAttachmentMarker, kReservedBitsOffset,
                             kReservedBits);
  bits = glm::bitfieldInsert(bits, encode(v.location), kLocationOffset,
                             kLocationBits);
  bits = glm::bitfieldInsert(bits, static_cast<uint32_t>(v.pipelineStage),
                             kPipelineStageOffset, kPipelineStageBits);
  return bits;
}
BindingInfo decodeBindingInfo(uint32_t bits) {
  return {
    .location = decodeLocation(
      glm::bitfieldExtract(bits, kLocationOffset, kLocationBits)),
    .pipelineStage = static_cast<PipelineStage>(
      glm::bitfieldExtract(bits, kPipelineStageOffset, kPipelineStageBits)),
  };
}

//
// TextureRead (17 bits):
//
// |   13 bits   |  2 bits  |  2 bits  |
// |   [0..12]   | [13..14] | [15..16] |
// | bindingInfo |   type   |  aspect  |

constexpr auto kTypeBits = 2;

constexpr auto kTypeOffset = kBindingInfoBits;
constexpr auto kTextureReadImageAspectOffset = kTypeOffset + kTypeBits;

[[nodiscard]] auto encode(const TextureRead &v) {
  uint32_t bits{0};
  bits = glm::bitfieldInsert(bits, encode(v.binding), 0, kBindingInfoBits);
  bits = glm::bitfieldInsert(bits, static_cast<uint32_t>(v.type), kTypeOffset,
                             kTypeBits);
  assert(v.imageAspect != rhi::ImageAspect::None);
  bits = glm::bitfieldInsert(bits, static_cast<uint32_t>(v.imageAspect),
                             kTextureReadImageAspectOffset, kImageAspectBits);
  return bits;
}
TextureRead decodeTextureRead(uint32_t bits) {
  return TextureRead{
    .binding = decodeBindingInfo(bits),
    .type = static_cast<TextureRead::Type>(
      glm::bitfieldExtract(bits, kTypeOffset, kTypeBits)),
    .imageAspect = static_cast<rhi::ImageAspect>(glm::bitfieldExtract(
      bits, kTextureReadImageAspectOffset, kImageAspectBits)),
  };
}

//
// ImageWrite (15 bits):
//
// |   13 bits   |  2 bits  |
// |   [0..12]   | [13..14] |
// | bindingInfo |  aspect  |

[[nodiscard]] auto encode(const ImageWrite &v) {
  uint32_t bits{0};
  bits = glm::bitfieldInsert(bits, encode(v.binding), 0, kBindingInfoBits);
  bits = glm::bitfieldInsert(bits, static_cast<uint32_t>(v.imageAspect),
                             kBindingInfoBits, kImageAspectBits);
  return bits;
}
ImageWrite decodeImageWrite(uint32_t bits) {
  return ImageWrite{
    .binding = decodeBindingInfo(bits),
    .imageAspect = static_cast<rhi::ImageAspect>(
      glm::bitfieldExtract(bits, kBindingInfoBits, kImageAspectBits)),
  };
}

//
// Conversion operators:
//

Attachment::operator uint32_t() const { return encode(*this); }
Location::operator uint32_t() const { return encode(*this); }
BindingInfo::operator uint32_t() const { return encode(*this); }
TextureRead::operator uint32_t() const { return encode(*this); }
ImageWrite::operator uint32_t() const { return encode(*this); }

} // namespace gfx
