// CNetGame. Observed strings/behavior in FUN_1000b290/1000af40/10008630/
// 1000aa60/1000a9d0/10008ad0/1000a230/10009df0. See header for mapping.
#include "samp/net/netgame.h"
#include "samp/net/packets.h"
#include "samp/net/bitstream.h"
#include "samp/net/sync.h"
#include "samp/net/limits.h"
#include "samp/game/pools.h"
#include "samp/game/vehicle.h"
#include "samp/common.h"
#include <windows.h>
#include <cstring>
#include <cstdio>

namespace samp { namespace net {

CNetGame::CNetGame() { strncpy(nick_, "Player", sizeof nick_ - 1); }

void CNetGame::SetServer(const char* host, uint16_t port, const char* pw) {
  if (host) strncpy(host_, host, sizeof host_ - 1);
  port_ = port ? port : 7777;
  if (pw) strncpy(password_, pw, sizeof password_ - 1);
}

void CNetGame::SetNick(const char* nick) {
  if (nick) strncpy(nick_, nick, sizeof nick_ - 1);
}

void CNetGame::SendConnectRetry() { // FUN_10008630: 3s timer
  uint32_t now = GetTickCount();
  if (now - lastTick_ > 3000) {
    char msg[128];
    snprintf(msg, sizeof msg, "Connecting to %s:%d...", host_, port_);
    peer_.Connect(host_, port_, password_[0] ? password_ : nullptr, (int)strlen(password_));
    lastTick_ = now;
    state_ = NetState::Connecting;
  }
}

void CNetGame::OnAccept_Join() { // FUN_1000aa60
  // Reference join (netgame.cpp:745-788): challenge_ ^= NETGAME_VERSION (4242),
  // then RPC_ClientJoin(version 4242, mod 0x01, nick, challenge, GPCI id, ver string).
  challenge_ ^= kNetGameVersion;
  BitStream bs;
  bs.WriteU32(0xFDE);            // observed magic
  bs.WriteStr(nick_);
  bs.WriteU32(0);                // client-hash placeholder (FUN_1005fa60 TODO(rea))
  bs.WriteStr(kVersion);         // "0.3.DL-R1"
  peer_.Send(bs, 1, 8, 0);       // priority=1, reliability=8, channel=0
  state_ = NetState::Joined;
}

void CNetGame::OnLost() { // FUN_1000a9d0
  peer_.Disconnect(0, 0);
  state_ = NetState::WaitConnect;
}

void CNetGame::OnServerClosed() { // FUN_10008ad0
  peer_.Disconnect(2000, 0);
}

void CNetGame::ResetWorld() { // FUN_1000a230
  state_ = NetState::Restarting;
}

void CNetGame::DispatchIncoming() { // FUN_1000af40 (abridged)
  for (;;) {
    RecvPacket* pkt = peer_.Receive();
    if (!pkt) break;
    uint8_t id = pkt->data ? pkt->data[0] : 0;
    if (id == '(' && pkt->length > 5) id = pkt->data[5]; // timestamped prefix
    switch (id) {
      case (uint8_t)ConnEvent::Accepted: OnAccept_Join(); break;
      case (uint8_t)ConnEvent::Lost: OnLost(); break;
      case (uint8_t)ConnEvent::ServerClosed: OnServerClosed(); break;
      case (uint8_t)ConnEvent::NoResponse:
      case (uint8_t)ConnEvent::ServerFull:
        state_ = NetState::WaitConnect; break;
      case (uint8_t)ConnEvent::BadPassword:
        peer_.Disconnect(0, 0); state_ = NetState::WaitConnect; break;
      case (uint8_t)ConnEvent::Banned:
      case (uint8_t)ConnEvent::EncryptFail:
        state_ = NetState::WaitConnect; break;
      default:
        // Sync IDs 200-212: count + route into game pools.
        if (pkt->data && pkt->length > 0)
          RouteSync(id, pkt->data, pkt->length);
        break;
    }
    peer_.Deallocate(pkt);
  }
}

void CNetGame::Pump() { // FUN_1000b290
  if (state_ == NetState::WaitConnect) SendConnectRetry();
  else DispatchIncoming();
}

std::string CNetGame::FormatStats(const NetStats& s) {
  char b[160];
  snprintf(b, sizeof b, "Up: %u pkts %llu B | Down: %u pkts %llu B",
           s.upPackets, s.upBytes, s.downPackets, s.downBytes);
  return b;
}

bool CNetGame::RouteSync(uint8_t id, const uint8_t* data, size_t len) {
  ++syncCount_[id];
  NoteRecv(len);
  if (!data) return false;
  if (id == (uint8_t)SampPacketId::VehicleSync) { // 200, FUN_1000a6f0 counterpart
    if (len < 2u + 3u * sizeof(float)) return false;
    uint16_t vid = (uint16_t)(data[0] | (data[1] << 8));
    float x, y, z;
    memcpy(&x, data + 2, 4); memcpy(&y, data + 6, 4); memcpy(&z, data + 10, 4);
    auto* slot = game::Vehicles().Get((int)vid);
    if (slot) {
      game::RemoteFrame f;
      f.pos[0] = x; f.pos[1] = y; f.pos[2] = z;
      f.airOrBoat = false;
      float cur[3] = {slot->pos[0], slot->pos[1], slot->pos[2]};
      float blended[3];
      game::BlendVehiclePos(cur, f, blended);
      slot->pos[0] = blended[0]; slot->pos[1] = blended[1]; slot->pos[2] = blended[2];
    }
    return true;
  }
  if (id == (uint8_t)SampPacketId::PlayerSync) { // 207: minimal keys+pos
    if (len < 3u * 2u + 3u * sizeof(float)) return false;
    last_.hasOnFoot = true;
    last_.keys = (uint16_t)(data[4] | (data[5] << 8));
    memcpy(&last_.x, data + 6, 4);
    memcpy(&last_.y, data + 10, 4);
    memcpy(&last_.z, data + 14, 4);
    return true;
  }
  if (id == (uint8_t)SampPacketId::AimSync) { // 203: full 31-byte AIM struct
    if (len < 31u) return false;
    BitStream bs(data, len, true);
    AimSync a;
    // NOTE: timestamped packets carry id at [5]; caller strips it.
    if (!DecodeAim(bs, a)) return false;
    last_.hasAim = true;
    last_.camMode = a.camMode;
    last_.aimZ = a.aimZ;
    return true;
  }
  if (id == (uint8_t)SampPacketId::PassengerSync || // 209 (minimal vehId+xyz)
      id == (uint8_t)SampPacketId::TrailerSync) {   // 211 (same minimal shape)
    if (len < 2u + 3u * sizeof(float)) return false;
    return true; // validated + counted; pool blend follows vehicle path later
  }
  return true; // counted; other IDs' struct parse is TODO(rea)
}

}} // namespace
