#pragma once
// Command window recall + registry. Reference: samp-source client/cmdwindow.h
// (MAX_CMDS 128, MAX_CMD_STRLEN 32, MAX_RECALLS 10, MAX_CMD_INPUT 128).
// Pure logic here; WndProc wiring stays in input/input.*.
#include <string>

namespace samp { namespace input {

inline constexpr int kMaxCmds = 128;
inline constexpr int kMaxCmdLen = 32;
inline constexpr int kMaxRecalls = 10;
inline constexpr int kMaxCmdInput = 128;

class CmdHistory {
public:
  void Push(const std::string& line); // stores truncated to kMaxCmdInput
  // Browse with UP (older) / DOWN (newer); empty when exhausted.
  std::string RecallUp();
  std::string RecallDown();
  int Size() const { return size_; }
private:
  std::string slots_[kMaxRecalls];
  int size_ = 0;  // filled slots (<= kMaxRecalls)
  int cursor_ = 0; // 0 = live line; 1..size_ = steps back
};

}} // namespace
