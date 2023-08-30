#pragma once

#include "rhi/Texture.hpp"
#include "MaterialEditor/Nodes/NodeCommon.hpp"
#include <memory>

struct TextureParam {
  std::shared_ptr<rhi::Texture> texture;

  template <class Archive> void serialize(Archive &) const {
    // Can not serialize right here.
    // A texture path must be relative to a parent (graph) file.
  }
};

bool inspectNode(int32_t id, std::optional<const char *> userLabel,
                 TextureParam &);

[[nodiscard]] NodeResult evaluate(MaterialGenerationContext &, int32_t id,
                                  std::optional<const char *> userLabel,
                                  const TextureParam &);
