#pragma once
// Chat window. Observed: FUN_10067c40 (render-to-surface w/ direct fallback),
// FUN_100c5e70 (root init, chatlog.txt), FUN_10068b50 (cmd reg),
// FUN_100c6950/69a0/69e0 (fontsize scaling), strings ChatWindow/directmode.
#include <string>
namespace samp { namespace gui {
class ChatWindow {
public:
  bool Init(void* device);
  void Print(const char* text);
  void SetFontSize(int delta); // -3..5 ("Valid fontsize: -3 to 5")
  void Render();
  void LogToFile(const std::string& line); // %s\chatlog.txt
private:
  bool directMode_ = false;
  int  fontDelta_ = 0;
};
ChatWindow& Chat();
}} // namespace
