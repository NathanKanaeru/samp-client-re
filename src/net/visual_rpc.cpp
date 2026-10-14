// Visual RPC builders. See visual_rpc.h.
#include "samp/net/visual_rpc.h"

namespace samp { namespace net {

static void PutText(BitStream& bs, const std::string& t) {
  bs.WriteU16((uint16_t)t.size());
  if (!t.empty()) bs.WriteBytes(t.data(), t.size());
}

void BuildShowTextDraw(BitStream& bs, const TextDrawDesc& d) {
  bs.WriteU16(d.id);
  bs.WriteBytes(&d.x, 4);
  bs.WriteBytes(&d.y, 4);
  PutText(bs, d.text);
  bs.WriteU32(d.color);
  bs.WriteU8(d.style);
  bs.WriteBytes(&d.sizeX, 4);
  bs.WriteBytes(&d.sizeY, 4);
}

void BuildHideTextDraw(BitStream& bs, uint16_t id) { bs.WriteU16(id); }

void BuildEditTextDraw(BitStream& bs, uint16_t id, float x, float y, const std::string& text) {
  bs.WriteU16(id);
  bs.WriteBytes(&x, 4);
  bs.WriteBytes(&y, 4);
  PutText(bs, text);
}

void BuildAddGangZone(BitStream& bs, float x1, float y1, float x2, float y2,
                      uint32_t color, uint32_t alt) {
  bs.WriteBytes(&x1, 4); bs.WriteBytes(&y1, 4);
  bs.WriteBytes(&x2, 4); bs.WriteBytes(&y2, 4);
  bs.WriteU32(color);
  bs.WriteU32(alt);
}

void BuildRemoveGangZone(BitStream& bs, uint16_t id) { bs.WriteU16(id); }
void BuildFlashGangZone(BitStream& bs, uint16_t id, uint32_t alt) {
  bs.WriteU16(id);
  bs.WriteU32(alt);
}
void BuildStopFlashGangZone(BitStream& bs, uint16_t id) { bs.WriteU16(id); }

}} // namespace
