#pragma once
// CNetGame. Observed: FUN_1000b290 Pump, FUN_1000af40 Dispatch, FUN_10008630
// SendConnectRetry, FUN_1000aa60 OnAccept_Join, FUN_1000a9d0 OnLost,
// FUN_10008ad0 OnClosed, FUN_1000a230 ResetWorld, FUN_10009df0 AuthReply.
#include <cstdint>
#include "samp/net/rakpeer.h"

namespace samp { namespace net {

enum class NetState : int {
  Idle = 0, WaitConnect = 1, Connecting = 2, Challenge = 5, Joined = 6, Restarting = 0xB,
};

class CNetGame {
public:
  CNetGame();
  void Pump();              // FUN_1000b290
  void SendConnectRetry();  // FUN_10008630 (3s timer, "Connecting to %s:%d...")
  void DispatchIncoming();  // FUN_1000af40
  void OnAccept_Join();     // FUN_1000aa60 ("Connected. Joining...", sends 0xFDE+nick+hash+"0.3.DL-R1")
  void OnLost();            // FUN_1000a9d0 ("Lost connection...")
  void OnServerClosed();    // FUN_10008ad0 ("Server closed the connection.")
  void ResetWorld();        // FUN_1000a230
  void SetServer(const char* host, uint16_t port, const char* password);
  NetState state() const { return state_; }
private:
  RakPeerWrapper peer_;
  char     host_[64] = {};
  uint16_t port_ = 7777;
  char     password_[64] = {};
  char     nick_[32] = {};
  NetState state_ = NetState::Idle;
  uint32_t lastTick_ = 0;
};

}} // namespace
