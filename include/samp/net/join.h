#pragma once
// ClientJoin packet (RPC_ClientJoin, RPC id 25). Field order ported from
// samp-source client/net/netgame.cpp Packet_ConnectionSucceeded (verified):
// int32 version, u8 mod, u8 nameLen + nick, u32 challenge^version,
// u8 cidLen + cid, u8 verLen + version bytes (incl NUL).
// NOTE DL-R1 delta: our REA on FUN_1000aa60 shows a 0xFDE-prefixed shape with
// "0.3.DL-R1" string instead — so THIS builder is the 0.3.7-shape reference;
// DL-R1 wire parity is TODO_VERIFY (no live DL server here).
#include <cstdint>
#include <string>
#include "samp/net/bitstream.h"
#include "samp/net/limits.h"

namespace samp { namespace net {

inline void BuildClientJoin(BitStream& bs, const std::string& nick,
                            uint32_t challengeXor, const std::string& cid,
                            const std::string& ver = kNetVersionString,
                            uint8_t mod = kModVersion) {
  bs.WriteU32(kNetGameVersion);
  bs.WriteU8(mod);
  bs.WriteU8((uint8_t)nick.size());
  bs.WriteBytes(nick.data(), nick.size());
  bs.WriteU32(challengeXor);
  bs.WriteU8((uint8_t)cid.size());
  bs.WriteBytes(cid.data(), cid.size());
  bs.WriteU8((uint8_t)(ver.size() + 1u));
  bs.WriteBytes(ver.data(), ver.size());
  uint8_t nul = 0;
  bs.WriteBytes(&nul, 1);
}

}} // namespace
