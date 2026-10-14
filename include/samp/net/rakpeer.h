#pragma once
// RakPeer wrapper. Observed: object at CNetGame+0x2c with vtable;
// slot[1]=Connect, [2]=Disconnect, [8]=Receive, [9]=Deallocate, Send slots.
// UDP core is statically linked (sendto/recvfrom have no resolved thunk callers;
// HTTP client FUN_100029d0 owns the only resolved send/connect callers).
// Default port 7777 / RPC wire format = UNKNOWN (TODO(rea)).
#include <cstdint>
#include <string>

namespace samp { namespace net {

class BitStream;
struct RecvPacket {
  uint8_t* data = nullptr; // packet bytes; byte[0]=id, timestamped => id at [5]
  uint32_t length = 0;
  uint32_t reserved = 0;   // sorry: keep layout simple
};

class RakPeerWrapper {
public:
  RakPeerWrapper();
  ~RakPeerWrapper();
  bool Connect(const char* host, uint16_t port, const char* pw, int pwLen);
  void Disconnect(uint32_t timeoutMs, uint8_t channel);
  RecvPacket* Receive();
  void Deallocate(RecvPacket* p);
  // reliability: observed join uses priority=1, reliability=8, channel=0
  bool Send(BitStream& bs, uint8_t priority, uint8_t reliability, uint8_t channel);
  bool IsActive() const { return active_; }
private:
  bool active_ = false;
  std::string host_;
  uint16_t port_ = 0;
  // TODO(rea): replace UDP stubs with real RakNet (e.g. SLikeNet) once
  // send-slot +0x18/+100 mapping is traced from vtable this+0x2c.
};

}} // namespace
