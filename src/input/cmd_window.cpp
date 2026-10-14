// Cmd history ring. See cmd_window.h.
#include "samp/input/cmd_window.h"

namespace samp { namespace input {

void CmdHistory::Push(const std::string& line) {
  std::string v = line.substr(0, kMaxCmdInput);
  if (size_ < kMaxRecalls) slots_[size_++] = v;
  else {
    for (int i = 0; i < kMaxRecalls - 1; ++i) slots_[i] = slots_[i + 1];
    slots_[kMaxRecalls - 1] = v;
  }
  cursor_ = 0;
}

std::string CmdHistory::RecallUp() {
  if (cursor_ < size_) ++cursor_;
  if (cursor_ == 0) return "";
  return slots_[size_ - cursor_];
}

std::string CmdHistory::RecallDown() {
  if (cursor_ > 0) --cursor_;
  if (cursor_ == 0) return "";
  return slots_[size_ - cursor_];
}

}} // namespace
