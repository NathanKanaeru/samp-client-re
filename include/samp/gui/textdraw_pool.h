#pragma once
// TextDraw runtime pool (2304 = 2048 global + 256 player). Reference:
// textdrawpool.h + TEXT_DRAW_TRANSMIT (shared.h:357-383), CTextDraw text 800.
// Pure slot logic here (D3D render stays out); show/hide state + tests.
#include <cstdint>
#include <string>
#include "samp/net/limits.h"

namespace samp { namespace gui {

struct TextDraw {
  std::string text;   // <= 800 (MAX_TEXT_DRAW_LINE)
  std::string str;    // formatted buffer (<= 3200)
  bool shown = false;
  bool hasBox = false;
};

class TextDrawPool {
public:
  bool Init(void*) { return true; } // device-bound fonts (D3D layer)
  TextDraw* Alloc();                    // -1 slot via index or nullptr
  int AllocId();
  bool Free(int id);
  TextDraw* Get(int id);
  bool SetText(uint32_t id, const char* text, uint32_t size); // size guard
  bool Show(int id);
  bool Hide(int id);
  int Shown() const;
private:
  TextDraw slots_[net::kMaxTextDrawSlots];
  bool used_[net::kMaxTextDrawSlots] = {};
};

TextDrawPool& TextDraws();

}} // namespace
