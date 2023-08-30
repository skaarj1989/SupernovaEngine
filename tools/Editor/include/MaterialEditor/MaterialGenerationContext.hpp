#pragma once

#include "renderer/Material.hpp"
#include "ShaderCodeComposer.hpp"
#include "ShaderToken.hpp"

#include <stack>
#include <map>
#include <array>

struct MaterialGenerationContext {
  const gfx::MaterialDomain materialDomain;

  struct Shader {
    const rhi::ShaderType type;
    std::stack<ShaderToken> tokens;
    ShaderCodeComposer composer;
  };
  Shader *currentShader{nullptr};

  template <typename T> using Map = std::map<std::string, T, std::less<>>;

  using PropertyMap = Map<gfx::Property::Value>;
  PropertyMap properties;
  using TextureMap = Map<std::shared_ptr<rhi::Texture>>;
  TextureMap textures;
};

// @return Top element of a given stack (top and pop).
[[nodiscard]] ShaderToken extractTop(std::stack<ShaderToken> &);

inline void extractN(std::stack<ShaderToken> &tokens, std::size_t count,
                     auto &out) {
  assert(count > 0 && tokens.size() >= count);
  std::generate_n(out.rbegin(), count,
                  [&tokens] { return extractTop(tokens); });
}

template <std::size_t _Size>
[[nodiscard]] auto extract(std::stack<ShaderToken> &tokens) {
  static_assert(_Size > 0);
  std::array<ShaderToken, _Size> arr;
  extractN(tokens, _Size, arr);
  return arr;
}

[[nodiscard]] std::vector<ShaderToken> extractN(std::stack<ShaderToken> &,
                                                std::size_t count);

[[nodiscard]] std::vector<DataType>
  extractArgTypes(std::span<const ShaderToken>);
[[nodiscard]] std::vector<std::string> transform(std::span<const DataType>);
[[nodiscard]] std::vector<std::string>
  extractArgNames(std::span<const ShaderToken>);

[[nodiscard]] std::string nodeIdToString(int32_t id);

[[nodiscard]] std::optional<std::string> assure(const ShaderToken &,
                                                DataType requiredType);

template <typename T, typename V>
std::string emplaceWithFallback(T &map,
                                const std::pair<std::string, std::string> &keys,
                                V &&v) {
  for (auto &key : {keys.first, keys.second}) {
    if (key.empty()) {
      continue;
    } else if (map.contains(key)) {
      return key;
    } else {
      auto [_, emplaced] = map.try_emplace(key, std::forward<V>(v));
      assert(emplaced);
      return key;
    }
  }
  assert(false);
  return "";
}
