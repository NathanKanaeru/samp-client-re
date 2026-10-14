#pragma once
// RPC dispatch framework. Handler IDs from rpc_ids.h (GameRpc 0-49,
// ScriptRpc 100-189). Client-side handlers with observable effects:
// Chat(21)/ClientMessage(12) -> ChatWindow, Death(32) -> DeathWindow via
// death_codec, WorldTime(13)/Weather(17)/SetTimeEx(19)/ToggleClock(20) ->
// WorldState. Unknown IDs count as unhandled (TODO(rea): full 90 handlers).
// Pure + tested with synthetic payloads.
#include <cstdint>
#include "samp/net/bitstream.h"
#include "samp/net/rpc_ids.h"

namespace samp { namespace net {

struct WorldState {
  uint8_t hour = 12, minute = 0;
  uint8_t weather = 10;
  bool clockOn = true;
};

struct RpcCtx {
  WorldState* world = nullptr;
  void* chat = nullptr;   // gui::ChatWindow* (opaque to avoid gui include)
  void* deaths = nullptr; // gui::DeathWindow*
  void (*chatAdd)(void* chat, const char* text, int type) = nullptr;
  void (*deathAdd)(void* dw, const char* killer, const char* killee, int reason) = nullptr;
};

using RpcHandler = bool (*)(BitStream& bs, RpcCtx& ctx);

class RpcDispatcher {
public:
  void Register(uint8_t id, RpcHandler h);
  void RegisterBuiltins(); // Chat/ClientMessage/Death/WorldTime/Weather/SetTimeEx/ToggleClock
  bool Dispatch(uint8_t id, BitStream& bs, RpcCtx& ctx);
  uint32_t handled() const { return handled_; }
  uint32_t unhandled() const { return unhandled_; }
private:
  RpcHandler table_[256] = {};
  uint32_t handled_ = 0, unhandled_ = 0;
};

}} // namespace
