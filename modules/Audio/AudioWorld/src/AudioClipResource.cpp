#include "AudioClipResource.hpp"

AudioClipResource::AudioClipResource(audio::Buffer &&buffer,
                                     const std::filesystem::path &p)
    : Resource{p}, audio::Buffer{std::move(buffer)} {}
