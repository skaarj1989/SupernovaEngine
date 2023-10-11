#include "AudioClipLoader.hpp"
#include "audio/Device.hpp"

#include "os/FileSystem.hpp"
#include "VorbisDecoder.hpp"
#include "WaveDecoder.hpp"

#include "spdlog/spdlog.h"

namespace {

[[nodiscard]] std::expected<audio::Buffer, std::string>
tryLoad(const std::filesystem::path &p, audio::Device &device) {
  auto decoder = createDecoder(p);
  if (!decoder) {
    return std::unexpected{decoder.error()};
  }
  const auto &info = (*decoder)->getInfo();
  const auto length = info.dataSize();
  std::vector<std::byte> buffer(length);
  (*decoder)->read(buffer.data(), buffer.size());
  return device.createBuffer(info, buffer.data());
}

} // namespace

AudioClipLoader::result_type
AudioClipLoader::operator()(const std::filesystem::path &p,
                            audio::Device &device) const {
  if (auto buffer = tryLoad(p, device); buffer) {
    return std::make_shared<AudioClipResource>(std::move(*buffer), p);
  } else {
    SPDLOG_ERROR("{}: Audio clip loading failed. {}",
                 os::FileSystem::relativeToRoot(p)->string(), buffer.error());
    return {};
  }
}

std::expected<std::unique_ptr<audio::Decoder>, std::string>
createDecoder(const std::filesystem::path &p) {
  const auto ext = os::FileSystem::getExtension(p);
  if (!ext) {
    return std::unexpected{"No extension."};
  }
  auto file = os::FileSystem::mapFile(p);
  if (!file) {
    return std::unexpected{"Could not open file."};
  }

  std::unique_ptr<audio::Decoder> decoder;
  try {
    if (ext == ".wav") {
      decoder = std::make_unique<audio::WaveDecoder>(std::move(file));
    } else if (ext == ".ogg") {
      decoder = std::make_unique<audio::VorbisDecoder>(std::move(file));
    }
  } catch (const std::exception &e) {
    return std::unexpected{e.what()};
  }

  if (!decoder) {
    return std::unexpected{std::format("Unsupported extension: '{}'", *ext)};
  }
  return decoder;
}
