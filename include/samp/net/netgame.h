#pragma once
// CNetGame. Observed: FUN_1000b290 Pump, FUN_1000af40 Dispatch, FUN_10008630
// SendConnectRetry, FUN_1000aa60 OnAccept_Join, FUN_1000a9d0 OnLost,
// FUN_10008ad0 OnClosed, FUN_1000a230 ResetWorld, FUN_10009df0 AuthReply.
// Reference join (samp-source netgame.cpp:745-788): retry every 3s via
// Connect(host,port,0,0,10); on ACCEPT read uiChallenge, xor with
// NETGAME_VERSION (4242), send RPC_ClientJoin(version 4242, mod 0x01,
// nick, challenge^version, GPCI computer id, version string).
#include <cstdint>
#include <string>
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
  void SetNick(const char* nick);
  NetState state() const { return state_; }
  uint32_t challenge() const { return challenge_; }
  // Sync router: counts packets per ID (netstats/F5) and applies the
  // documented-minimal layouts. Wire offsets for DL-R1 sync structs are
  // inferred from the R5 PACKET200 doc — full struct parity is TODO(rea).
  // Minimal 200 layout: u16 vehicleId + float x,y,z. Returns false when
  // the buffer is too short for the claimed ID.
  bool RouteSync(uint8_t id, const uint8_t* data, size_t len);
  uint32_t syncCount(uint8_t id) const { return syncCount_[id]; }
  // Last validated onfoot/aim frames (207/203), for HUD/debug. Zeroed init.
  struct LastFrames {
    bool hasOnFoot = false;
    uint16_t keys = 0;
    float x = 0, y = 0, z = 0;
    bool hasAim = false;
    uint8_t camMode = 0;
    float aimZ = 0;
  };
  const LastFrames& lastFrames() const { return last_; }
  // Transport accounting for the F5 overlay (libsamp format_transport_statistics
  // shape: rates + totals). Pure + tested.
  struct NetStats {
    uint64_t upBytes = 0, downBytes = 0;
    uint32_t upPackets = 0, downPackets = 0;
  };
  const NetStats& stats() const { return stats_; }
  void NoteSent(size_t n) { stats_.upBytes += n; ++stats_.upPackets; }
  void NoteRecv(size_t n) { stats_.downBytes += n; ++stats_.downPackets; }
  static std::string FormatStats(const NetStats& s);
private:
  RakPeerWrapper peer_;
  char     host_[64] = {};
  uint16_t port_ = 7777;
  char     password_[64] = {};
  char     nick_[32] = {};
  NetState state_ = NetState::Idle;
  uint32_t lastTick_ = 0;
  uint32_t challenge_ = 0; // server challenge, xor NETGAME_VERSION on join
  uint32_t syncCount_[256] = {};
  NetStats stats_;
  LastFrames last_;
};

}} // namespace
