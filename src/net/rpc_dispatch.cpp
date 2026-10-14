// RPC dispatch. See rpc_dispatch.h.
#include "samp/net/rpc_dispatch.h"
#include "samp/net/death_codec.h"
#include <cstring>
#include <cstdio>

namespace samp { namespace net {

static bool ReadPString(BitStream& bs, char* out, size_t cap, uint8_t& lenOut) {
  uint8_t n = 0;
  if (!bs.ReadU8(n) || n >= cap) return false;
  if (!bs.ReadBytes(out, n)) return false;
  out[n] = 0;
  lenOut = n;
  return true;
}

static bool HChat(BitStream& bs, RpcCtx& ctx) { // RPC 21: u8 len + text
  char text[256];
  uint8_t n = 0;
  if (!ReadPString(bs, text, sizeof text, n)) return false;
  if (ctx.chat && ctx.chatAdd) ctx.chatAdd(ctx.chat, text, 0);
  return true;
}

static bool HClientMessage(BitStream& bs, RpcCtx& ctx) { // RPC 12: u32 color + u8 len + text
  uint32_t color = 0;
  char text[256];
  uint8_t n = 0;
  if (!bs.ReadU32(color) || !ReadPString(bs, text, sizeof text, n)) return false;
  if (ctx.chat && ctx.chatAdd) ctx.chatAdd(ctx.chat, text, 1);
  return true;
}

static bool HDeath(BitStream& bs, RpcCtx& ctx) { // RPC 32: killer u16 + killee u16 + reason u8 (+ names unknown here)
  uint8_t raw[5];
  if (!bs.ReadBytes(raw, 5)) return false;
  DeathMessage dm;
  if (!DecodeDeathMessage(raw, 5, &dm)) return false;
  if (ctx.deaths && ctx.deathAdd) {
    char a[16], b[16];
    snprintf(a, sizeof a, "player%u", dm.killer);
    snprintf(b, sizeof b, "player%u", dm.killee);
    ctx.deathAdd(ctx.deaths, a, b, dm.reason);
  }
  return true;
}

static bool HWorldTime(BitStream& bs, RpcCtx& ctx) { // RPC 13: u8 hour, u8 minute
  uint8_t h = 0, m = 0;
  if (!bs.ReadU8(h) || !bs.ReadU8(m)) return false;
  if (ctx.world) { ctx.world->hour = h; ctx.world->minute = m; }
  return true;
}

static bool HWeather(BitStream& bs, RpcCtx& ctx) { // RPC 17: u8 weather
  uint8_t w = 0;
  if (!bs.ReadU8(w)) return false;
  if (ctx.world) ctx.world->weather = w;
  return true;
}

static bool HSetTimeEx(BitStream& bs, RpcCtx& ctx) { // RPC 19: same as WorldTime
  return HWorldTime(bs, ctx);
}

static bool HToggleClock(BitStream& bs, RpcCtx& ctx) { // RPC 20: u8 on
  uint8_t on = 0;
  if (!bs.ReadU8(on)) return false;
  if (ctx.world) ctx.world->clockOn = on != 0;
  return true;
}

void RpcDispatcher::Register(uint8_t id, RpcHandler h) { table_[id] = h; }

void RpcDispatcher::RegisterBuiltins() {
  Register((uint8_t)GameRpc::Chat, HChat);
  Register((uint8_t)GameRpc::ClientMessage, HClientMessage);
  Register((uint8_t)GameRpc::Death, HDeath);
  Register((uint8_t)GameRpc::WorldTime, HWorldTime);
  Register((uint8_t)GameRpc::Weather, HWeather);
  Register((uint8_t)GameRpc::SetTimeEx, HSetTimeEx);
  Register((uint8_t)GameRpc::ToggleClock, HToggleClock);
}

bool RpcDispatcher::Dispatch(uint8_t id, BitStream& bs, RpcCtx& ctx) {
  RpcHandler h = table_[id];
  if (!h) { ++unhandled_; return false; }
  bool ok = h(bs, ctx);
  if (ok) ++handled_;
  else ++unhandled_;
  return ok;
}

}} // namespace
