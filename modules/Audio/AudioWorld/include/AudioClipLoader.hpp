#pragma once

#include "AudioClipResource.hpp"
#include "entt/resource/loader.hpp"
#include "Decoder.hpp"
#include <expected>

using AudioClipResourceHandle = entt::resource<AudioClipResource>;

struct AudioClipLoader final : entt::resource_loader<AudioClipResource> {
  result_type operator()(const std::filesystem::path &, audio::Device &) const;
};

[[nodiscard]] std::expected<std::unique_ptr<audio::Decoder>, std::string>
createDecoder(const std::filesystem::path &);
