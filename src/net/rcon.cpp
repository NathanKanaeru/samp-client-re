// RCON session. See rcon.h.
#include "samp/net/rcon.h"

namespace samp { namespace net {

bool RconSession::Enqueue(const std::string& cmd) {
  if (cmd.empty() || (int)queue.size() >= kMaxQueue) return false;
  queue.push_back(cmd);
  return true;
}

bool RconSession::NextToSend(std::string& out) {
  if (queue.empty()) return false;
  out = queue.front();
  queue.erase(queue.begin());
  pending = out;
  return true;
}

void RconSession::OnReplyLine(const std::string& line, uint32_t nowTick) {
  if (!awaiting || nowTick - sentTick >= kReplyWindowMs) return;
  if (!reply.empty()) reply += "\n";
  reply += line;
}

std::vector<std::string> SplitReplyLines(const std::string& blob) {
  std::vector<std::string> out;
  size_t pos = 0;
  while (pos <= blob.size()) {
    size_t nl = blob.find('\n', pos);
    std::string line = blob.substr(pos, nl == std::string::npos ? nl : nl - pos);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    out.push_back(line);
    if (nl == std::string::npos) break;
    pos = nl + 1;
  }
  return out;
}

}} // namespace
