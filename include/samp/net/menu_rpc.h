#pragma once
// Menu select/quit responses (RPC MenuSelect 37 / MenuQuit 38).
// Builder shapes: select = u8 menuId + u8 row; quit = u8 menuId.
// Pure + tested. Server-side menu content lives in game/slots.h MenuPool.
#include <cstdint>
#include "samp/net/bitstream.h"

namespace samp { namespace net {

inline void BuildMenuSelect(BitStream& bs, uint8_t menu, uint8_t row) {
  bs.WriteU8(menu);
  bs.WriteU8(row);
}

inline void BuildMenuQuit(BitStream& bs, uint8_t menu) { bs.WriteU8(menu); }

}} // namespace
