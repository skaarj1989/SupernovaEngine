#include "catch.hpp"

#include "os/FileSystem.hpp"
#include "audio/Device.hpp"
#include "audio/Source.hpp"
#include "audio/StreamPlayer.hpp"

#include "VorbisDecoder.hpp"
#include "WaveDecoder.hpp"

#include <thread>

using namespace std::chrono_literals;

template <class T>
concept DecoderType = std::is_base_of_v<audio::Decoder, T>;

template <DecoderType T>
[[nodiscard]] audio::Buffer load(audio::Device &device,
                                 const std::filesystem::path &p) {
  T decoder{os::FileSystem::mapFile(p)};
  REQUIRE(decoder.isOpen());
  const auto &info = decoder.getInfo();
  const auto length = info.dataSize();
  std::vector<std::byte> buffer(length);
  REQUIRE(decoder.read(buffer.data(), buffer.size()) == length);
  return device.createBuffer(info, buffer.data());
}
template <DecoderType T> void playback(const std::filesystem::path &p) {
  audio::Device audioDevice{};

  auto clip = std::make_shared<audio::Buffer>(load<T>(audioDevice, p));
  audio::Source source{std::move(clip)};
  source.play();
  while (source.isPlaying())
    ;
}

template <DecoderType T> void stream(const std::filesystem::path &p) {
  audio::Device audioDevice;

  audio::StreamPlayer player{2};
  auto decoder = std::make_unique<T>(os::FileSystem::mapFile(p));
  REQUIRE(decoder->isOpen());
  player.setStream(std::move(decoder), 250ms);
  player.play();

  do {
    player.update();
    std::this_thread::sleep_for(16ms);
  } while (player.isPlaying());
}

const std::filesystem::path kAssetDir{"D:/pack/audio/"};

TEST_CASE("PlaybackOgg") {
  playback<audio::VorbisDecoder>(kAssetDir / "stereo.ogg");
}
TEST_CASE("PlaybackWave") {
  playback<audio::WaveDecoder>(kAssetDir / "stereo.wav");
}
TEST_CASE("StreamOgg") {
  stream<audio::VorbisDecoder>(kAssetDir / "stereo.ogg");
}
TEST_CASE("StreamWave") {
  stream<audio::WaveDecoder>(kAssetDir / "stereo.wav");
}

int main(int argc, char *argv[]) { return Catch::Session{}.run(argc, argv); }
