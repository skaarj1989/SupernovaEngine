#pragma once

class RenderDoc {
public:
  RenderDoc();
  virtual ~RenderDoc() = default;

  bool hasRenderDoc() const;
  void captureFrame();

protected:
  void _beginFrame();
  void _endFrame();

private:
  bool m_wantCaptureFrame{false};
};
