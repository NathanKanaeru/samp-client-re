#pragma once
// RCON console client. Request bytes via BuildRcon (query.h, 'c' opcode);
// replies arrive as server chat/message lines (no sequence numbers on the
// wire, so matching is by recency window, like the original console).
// Pure queue + reply accumulator + timeout; socket IO stays out. Tested.
#include <cstdint>
#include <string>
#include <vector>

namespace samp { namespace net {

struct RconSession {
  static constexpr int kMaxQueue = 8;
  static constexpr uint32_t kReplyWindowMs = 2000;
  std::string pending;            // commands awaiting send
  std::vector<std::string> queue; // FIFO overflow guard
  std::string reply;              // accumulated lines of last command
  uint32_t sentTick = 0;
  bool awaiting = false;

  bool Enqueue(const std::string& cmd); // false when full/empty
  bool NextToSend(std::string& out);    // pops oldest into out
  void MarkSent(uint32_t nowTick) { awaiting = true; sentTick = nowTick; reply.clear(); }
  void OnReplyLine(const std::string& line, uint32_t nowTick); // appends in-window
  bool Expired(uint32_t nowTick) const { return awaiting && nowTick - sentTick >= kReplyWindowMs; }
  void Reset() { queue.clear(); reply.clear(); awaiting = false; }
};

// Splits a multi-line reply blob into lines (handles \r\n and \n).
std::vector<std::string> SplitReplyLines(const std::string& blob);

}} // namespace
