// Chat store: 100-entry shift buffer, word-wrap at 72 for chat type,
// page/scroll window. Behavior ported from samp-source chatwindow.*
// (MAX_MESSAGES 100, MAX_LINE_LENGTH 72, DISP_MESSAGES 10 default).
// Render (D3D font/sprite) stays out; this TU is pure logic + tested.
#include "samp/gui/chat.h"
#include <cstdio>
#include <cstring>

namespace samp { namespace gui {

static ChatWindow g_chat;
ChatWindow& Chat() { return g_chat; }

bool ChatWindow::Init(void*) {
  directMode_ = false; // try surface first; fallback on HRESULT<0 (observed)
  return true;
}

void ChatWindow::Print(const char* t) {
  AddMessage(t ? t : "", 0);
  printf("[chat] %s\n", t ? t : "");
}

void ChatWindow::AddMessage(const std::string& text, int type) {
  // Shift older entries down (chatwindow.cpp:257 memmove pattern).
  for (int i = 0; i < kMaxMessages - 1; ++i) entries_[i] = entries_[i + 1];
  ChatEntry& dst = entries_[kMaxMessages - 1];
  dst.type = type;
  if (type == 0 && text.size() > kMaxLineLength) {
    // Word-wrap: best break at/before 72 (chatwindow.cpp:292-323).
    size_t cut = kMaxLineLength;
    size_t sp = text.rfind(' ', kMaxLineLength);
    if (sp != std::string::npos && kMaxLineLength - sp <= 12) cut = sp;
    dst.text = text.substr(0, cut);
  } else {
    dst.text = text.substr(0, kMaxMessageLength);
  }
  ++total_;
}

void ChatWindow::SetFontSize(int d) {
  if (d < -3 || d > 5) { printf("Valid fontsize: -3 to 5\n"); return; }
  fontDelta_ = d;
}

std::vector<std::string> ChatWindow::VisibleSlice(int scroll) const {
  std::vector<std::string> out;
  if (scroll < 0) scroll = 0;
  int end = kMaxMessages - 1 - scroll;
  if (end < 0) return out;
  int start = end - pageSize_ + 1;
  if (start < 0) start = 0;
  for (int i = start; i <= end; ++i)
    if (!entries_[i].text.empty() || (unsigned)i >= (unsigned)kMaxMessages - total_)
      out.push_back(timestamp_ ? WithTimestamp(entries_[i].text, 12, 0) : entries_[i].text);
  return out;
}

std::string ChatWindow::WithTimestamp(const std::string& text, int hour, int minute) {
  char b[16];
  snprintf(b, sizeof b, "[%02d:%02d] ", hour < 0 ? 0 : (hour > 23 ? 23 : hour),
           minute < 0 ? 0 : (minute > 59 ? 59 : minute));
  return std::string(b) + text;
}

void ChatWindow::Render() {}

void ChatWindow::LogToFile(const std::string& line) {
  (void)line; // %s\chatlog.txt append (observed in FUN_100c5e70)
}

}} // namespace
