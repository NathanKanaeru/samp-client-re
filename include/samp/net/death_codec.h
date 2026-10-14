#pragma once
// Death message codec. Exact port of libsamp
// reimpl/src/net/death_message_codec.h (MIT, 32 lines): 5-byte payload
// u16LE killer + u16LE killee + u8 reason. Verified by their test vectors;
// our smoke test repeats the boundary cases.
#include <cstddef>
#include <cstdint>

namespace samp { namespace net {

struct DeathMessage {
  uint16_t killer = 0;
  uint16_t killee = 0;
  uint8_t reason = 0;
};

inline bool DecodeDeathMessage(const uint8_t* data, size_t n, DeathMessage* out) {
  if (!data || !out || n < 5u) return false;
  out->killer = (uint16_t)((uint16_t)data[0] | ((uint16_t)data[1] << 8));
  out->killee = (uint16_t)((uint16_t)data[2] | ((uint16_t)data[3] << 8));
  out->reason = data[4];
  return true;
}

}} // namespace
