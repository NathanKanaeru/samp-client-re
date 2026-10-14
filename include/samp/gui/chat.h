#pragma once
// Chat window. Observed: FUN_10067c40 (render-to-surface w/ direct fallback),
// FUN_100c5e70 (root init, chatlog.txt), FUN_10068b50 (cmd reg),
// FUN_100c6950/69a0/69e0 (fontsize scaling), strings ChatWindow/directmode.
// Reference (chatwindow.h): MAX_MESSAGE_LENGTH 144, MAX_LINE_LENGTH 72,
// MAX_MESSAGES 100, DISP_MESSAGES 10 (default page size).
#include <string>

namespace samp { namespace gui {

inline constexpr int kMaxMessageLength = 144;
inline constexpr int kMaxLineLength = 72;
inline constexpr int kMaxMessages = 100;
inline constexpr int kDispMessages = 10;

struct ChatEntry {
  std::string text;
  int type = 0; // 0 = chat (word-wrapped at 72), else raw up to 144
};

class ChatWindow {
public:
  bool Init(void* device);
  void Print(const char* text);
  void AddMessage(const std::string& text, int type);
  void SetFontSize(int delta); // -3..5 ("Valid fontsize: -3 to 5")
  void SetPageSize(int n) { pageSize_ = n < 1 ? 1 : (n > kMaxMessages ? kMaxMessages : n); }
  int PageSize() const { return pageSize_; }
  void Render();
  void LogToFile(const std::string& line); // %s\chatlog.txt
  const ChatEntry& Latest() const { return entries_[kMaxMessages - 1]; }
  unsigned Total() const { return total_; }
private:
  bool directMode_ = false;
  int fontDelta_ = 0;
  int pageSize_ = kDispMessages;
  ChatEntry entries_[kMaxMessages];
  unsigned total_ = 0;
};
ChatWindow& Chat();
}} // namespace
