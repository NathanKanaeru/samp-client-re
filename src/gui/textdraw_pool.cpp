// TextDraw pool + chat bubbles. See headers.
#include "samp/gui/textdraw_pool.h"
#include "samp/gui/bubbles.h"
#include <cstdio>

namespace samp { namespace gui {

static TextDrawPool g_td;
TextDrawPool& TextDraws() { return g_td; }

TextDraw* TextDrawPool::Alloc() {
  for (int i = 0; i < net::kMaxTextDrawSlots; ++i)
    if (!used_[i]) { used_[i] = true; slots_[i] = TextDraw(); return &slots_[i]; }
  return nullptr;
}

int TextDrawPool::AllocId() {
  for (int i = 0; i < net::kMaxTextDrawSlots; ++i)
    if (!used_[i]) { used_[i] = true; slots_[i] = TextDraw(); return i; }
  return -1;
}

bool TextDrawPool::Free(int id) {
  if (id < 0 || id >= net::kMaxTextDrawSlots || !used_[id]) return false;
  used_[id] = false; slots_[id] = TextDraw(); return true;
}

TextDraw* TextDrawPool::Get(int id) {
  if (id < 0 || id >= net::kMaxTextDrawSlots || !used_[id]) return nullptr;
  return &slots_[id];
}

bool TextDrawPool::SetText(uint32_t id, const char* text, uint32_t size) {
  TextDraw* t = Get((int)id);
  if (!t || !text) return false;
  if (size > 0x1000) { printf("Warning: ignoring large TextDraw size=%u\n", size); return false; }
  t->text.assign(text, size > 800 ? 800 : (size_t)size);
  return true;
}

bool TextDrawPool::Show(int id) {
  TextDraw* t = Get(id);
  if (!t) return false;
  t->shown = true; return true;
}

bool TextDrawPool::Hide(int id) {
  TextDraw* t = Get(id);
  if (!t) return false;
  t->shown = false; return true;
}

int TextDrawPool::Shown() const {
  int n = 0;
  for (int i = 0; i < net::kMaxTextDrawSlots; ++i)
    if (used_[i] && slots_[i].shown) ++n;
  return n;
}

bool ChatBubbles::Set(int player, const std::string& text, uint32_t nowTick) {
  if (player < 0 || player >= net::kMaxPlayers) return false;
  slots_[player].text = text.substr(0, kMaxBubbleText);
  slots_[player].setTick = nowTick;
  slots_[player].on = true;
  return true;
}

void ChatBubbles::Clear(int player) {
  if (player < 0 || player >= net::kMaxPlayers) return;
  slots_[player] = Bubble();
}

void ChatBubbles::Expire(uint32_t nowTick, uint32_t ttlMs) {
  for (int i = 0; i < net::kMaxPlayers; ++i)
    if (slots_[i].on && nowTick - slots_[i].setTick >= ttlMs) slots_[i].on = false;
}

const Bubble* ChatBubbles::Get(int player) const {
  if (player < 0 || player >= net::kMaxPlayers || !slots_[player].on) return nullptr;
  return &slots_[player];
}

int ChatBubbles::Active() const {
  int n = 0;
  for (int i = 0; i < net::kMaxPlayers; ++i)
    if (slots_[i].on) ++n;
  return n;
}

}} // namespace
