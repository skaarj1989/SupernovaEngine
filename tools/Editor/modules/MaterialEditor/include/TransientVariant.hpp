#pragma once

#include "ValueVariant.hpp"
#include "Attribute.hpp"
#include "BuiltInConstants.hpp"
#include "FrameBlockMember.hpp"
#include "CameraBlockMember.hpp"
#include "TextureParam.hpp"

// clang-format off
using TransientVariant = std::variant<
  std::monostate,
  ValueVariant,
  Attribute,
  BuiltInConstant, BuiltInSampler,
  FrameBlockMember, CameraBlockMember,
  TextureParam
>;
// clang-format on

[[nodiscard]] DataType getDataType(const TransientVariant &);
