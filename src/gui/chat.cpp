// Chat. Observed: FUN_10067c40 surface/direct, FUN_100c5e70 init,
// FUN_10068b50 cmd reg, fontsize scaling FUN_100c6950.
#include "samp/gui/chat.h"
#include <cstdio>

namespace samp { namespace gui {

static ChatWindow g_chat;
ChatWindow& Chat() { return g_chat; }

bool ChatWindow::Init(void*) {
  directMode_ = false; // try surface first; fallback on HRESULT<0 (observed)
  return true;
}
void ChatWindow::Print(const char* t) { printf("[chat] %s\n", t); }
void ChatWindow::SetFontSize(int d) {
  if (d < -3 || d > 5) { printf("Valid fontsize: -3 to 5\n"); return; }
  fontDelta_ = d;
}
void ChatWindow::Render() {}
void ChatWindow::LogToFile(const std::string& line) {
  (void)line; // %s\chatlog.txt append (observed in FUN_100c5e70)
}

}} // namespace
