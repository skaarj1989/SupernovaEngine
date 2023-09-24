#include "RmlUiPlatformInterface.hpp"
#include "os/FileSystem.hpp"

//
// RmlUiSystemInterface class:
//

RmlUiSystemInterface::RmlUiSystemInterface() { m_start = clock::now(); }

double RmlUiSystemInterface::GetElapsedTime() {
  using fsec = std::chrono::duration<double>;
  return fsec{clock::now() - m_start}.count();
}

//
// RmlUiFileInterface class:
//

Rml::FileHandle RmlUiFileInterface::Open(const Rml::String &path) {
  auto f = os::FileSystem::mapFile(os::FileSystem::getRoot() / path);
  return Rml::FileHandle(f.release());
}
void RmlUiFileInterface::Close(Rml::FileHandle file) {
  auto *f = std::bit_cast<os::DataStream *>(file);
  f->close();
  delete f;
}
size_t RmlUiFileInterface::Read(void *buffer, size_t size,
                                Rml::FileHandle file) {
  return std::bit_cast<os::DataStream *>(file)->read(buffer, size);
}
bool RmlUiFileInterface::Seek(Rml::FileHandle file, long offset, int origin) {
  return std::bit_cast<os::DataStream *>(file)->seek(
           offset, static_cast<os::DataStream::Origin>(origin)) == 0;
}
size_t RmlUiFileInterface::Tell(Rml::FileHandle file) {
  return std::bit_cast<os::DataStream *>(file)->tell();
}
