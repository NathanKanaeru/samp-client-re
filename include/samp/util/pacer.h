#pragma once
// Frame pacer for the fpslimit command (reference: fpslimit cmdproc +
// frame limiter GTA 0x00C1704C). Pure math: minimum frame interval from
// target FPS; Due() tells the loop whether to render this tick.
#include <cstdint>

namespace samp { namespace util {

class FramePacer {
public:
  void SetFps(int fps) { fps_ = fps < 1 ? 1 : (fps > 1000 ? 1000 : fps); }
  int Fps() const { return fps_; }
  // Minimum whole milliseconds per frame (90 FPS -> 11ms).
  uint32_t FrameMs() const { return 1000u / (uint32_t)fps_; }
  // True when nowMs-last >= FrameMs (first call always true).
  bool Due(uint32_t nowMs, uint32_t& last) {
    if (last == 0 || nowMs - last >= FrameMs()) { last = nowMs; return true; }
    return false;
  }
private:
  int fps_ = 60;
};

}} // namespace
