#pragma once
// TextDraw + gang-zone RPC builders (server-bound visuals).
// RPC ids: ShowTextDraw 160 / HideTextDraw 161 / EditTextDraw 162,
// AddGangZone 163 / RemoveGangZone 164 / FlashGangZone 165 / StopFlash 166.
// Payload shapes mirror TEXT_DRAW_TRANSMIT/GANG_ZONE structs; exact
// field order per shared.h. Pure builders + tested.
#include <cstdint>
#include <string>
#include "samp/net/bitstream.h"

namespace samp { namespace net {

struct TextDrawDesc {
  uint16_t id = 0;
  float x = 0, y = 0;
  std::string text; // u16 len + bytes
  uint32_t color = 0xFFFFFFFFu;
  uint8_t style = 1; // 0-3 font id
  float sizeX = 1.0f, sizeY = 1.0f;
};

// Show (full desc) / Hide (u16 id) / Edit position+text (u16 id + xy + text).
void BuildShowTextDraw(BitStream& bs, const TextDrawDesc& d);
void BuildHideTextDraw(BitStream& bs, uint16_t id);
void BuildEditTextDraw(BitStream& bs, uint16_t id, float x, float y, const std::string& text);

// Gang zone add (float x1,y1,x2,y2 + u32 color + u32 alt) / remove (u16 id) /
// flash (u16 id + u32 alt) — client keeps alt in pool while flashing.
void BuildAddGangZone(BitStream& bs, float x1, float y1, float x2, float y2,
                      uint32_t color, uint32_t alt);
void BuildRemoveGangZone(BitStream& bs, uint16_t id);
void BuildFlashGangZone(BitStream& bs, uint16_t id, uint32_t alt);
void BuildStopFlashGangZone(BitStream& bs, uint16_t id);

}} // namespace
