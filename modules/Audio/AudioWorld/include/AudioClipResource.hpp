#pragma once

#include "Resource.hpp"
#include "audio/Buffer.hpp"

class AudioClipResource : public Resource, public audio::Buffer {
public:
  AudioClipResource() = default;
  AudioClipResource(audio::Buffer &&, const std::filesystem::path &);
  AudioClipResource(const AudioClipResource &) = delete;
  AudioClipResource(AudioClipResource &&) noexcept = default;

  AudioClipResource &operator=(const AudioClipResource &) = delete;
  AudioClipResource &operator=(AudioClipResource &&) noexcept = default;
};
