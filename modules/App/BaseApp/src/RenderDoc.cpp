#include "RenderDoc.hpp"
#include "renderdoc_app.h"
#if WIN32
#  include <Windows.h>
#elif defined __linux
#  include <dlfcn.h>
#endif
#include <bit> // bit_cast

RENDERDOC_API_1_6_0 *g_renderdocAPI = nullptr;

//
// RenderDoc integration class:
//

RenderDoc::RenderDoc() {
#if WIN32
  if (auto hModule = GetModuleHandleA("renderdoc.dll"); hModule) {
    if (auto GetAPI = std::bit_cast<pRENDERDOC_GetAPI>(
          GetProcAddress(hModule, "RENDERDOC_GetAPI"));
        GetAPI) {
      GetAPI(eRENDERDOC_API_Version_1_6_0,
             std::bit_cast<void **>(&g_renderdocAPI));
    }
  }
#elif defined __linux
  if (auto *module = dlopen("librenderdoc.so", RTLD_NOW | RTLD_NOLOAD);
      module) {
    if (auto GetAPI =
          std::bit_cast<pRENDERDOC_GetAPI>(dlsym(module, "RENDERDOC_GetAPI"));
        GetAPI) {
      GetAPI(eRENDERDOC_API_Version_1_6_0,
             std::bit_cast<void **>(&g_renderdocAPI));
    }
  }
#endif
  if (g_renderdocAPI) g_renderdocAPI->MaskOverlayBits(0, 0);
}

bool RenderDoc::hasRenderDoc() const { return g_renderdocAPI != nullptr; }
void RenderDoc::captureFrame() {
  if (g_renderdocAPI && g_renderdocAPI->IsFrameCapturing() == 0) {
    m_wantCaptureFrame = true;
  }
}

//
// (private):
//

void RenderDoc::_beginFrame() {
  if (hasRenderDoc() && m_wantCaptureFrame) {
    g_renderdocAPI->StartFrameCapture(nullptr, nullptr);
  }
}
void RenderDoc::_endFrame() {
  if (hasRenderDoc() && m_wantCaptureFrame) {
    g_renderdocAPI->EndFrameCapture(nullptr, nullptr);
    m_wantCaptureFrame = false;
  }
}
