#pragma once
// Chat bubbles (per-player, 144 chars). Reference: chatbubble.h
// m_ChatBubbleEntries[MAX_PLAYERS 200], MAX_CHAT_BUBBLE_TEXT 144.
// Pure map logic + expiry tick; tested.
#include <cstdint>
#include <string>
#include "samp/net/limits.h"

namespace samp { namespace gui {

inline constexpr int kMaxBubbleText = 144;

struct Bubble { std::string text; uint32_t setTick = 0; bool on = false; };

class ChatBubbles {
public:
  bool Set(int player, const std::string& text, uint32_t nowTick);
  void Clear(int player);
  void Expire(uint32_t nowTick, uint32_t ttlMs); // turns off stale entries
  const Bubble* Get(int player) const;
  int Active() const;
private:
  Bubble slots_[net::kMaxPlayers];
};

}} // namespace
