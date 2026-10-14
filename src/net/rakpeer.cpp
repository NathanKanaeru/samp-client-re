// RakPeerWrapper + API registry. Observed: FUN_100203f0/FUN_10020260,
// FUN_10034cd0 method list, FUN_10030db0 subscribe, FUN_10036a00 console pw.
// Real UDP transport = TODO(rea) — stubs keep link + tests green.
#include "samp/net/rakpeer.h"
#include "samp/net/rpc.h"
#include "samp/net/bitstream.h"
#include <cstring>

namespace samp { namespace net {

RakPeerWrapper::RakPeerWrapper() = default;
RakPeerWrapper::~RakPeerWrapper() = default;

bool RakPeerWrapper::Connect(const char* host, uint16_t port, const char*, int) {
  host_ = host ? host : ""; port_ = port; active_ = true;
  return true;
}
void RakPeerWrapper::Disconnect(uint32_t, uint8_t) { active_ = false; }
RecvPacket* RakPeerWrapper::Receive() { return nullptr; } // TODO(rea): UDP pump
void RakPeerWrapper::Deallocate(RecvPacket* p) { delete p; }
bool RakPeerWrapper::Send(BitStream&, uint8_t, uint8_t, uint8_t) {
  return active_; // TODO(rea): wire Reliability(8)/Priority(1)/Channel(0)
}

static std::vector<ApiEntry>& Reg() { static std::vector<ApiEntry> v; return v; }
void RegisterApi(int argc, const char* name, const char* sig) {
  Reg().push_back({argc, name ? name : "", sig ? sig : ""});
}
const std::vector<ApiEntry>& ApiRegistry() { return Reg(); }

void RegisterRakPeerApi() {
  static bool done = false;
  if (done) return; done = true;
  const char* names[] = {"Initialize","InitializeSecurity","Connect","Disconnect",
    "SetIncomingPassword","GetIncomingPassword","Send","Receive","Deallocate",
    "Ping","GetAveragePing","AdvertiseSystem","ApplyNetworkSimulator", nullptr};
  for (int i = 0; names[i]; ++i) RegisterApi(0, names[i], "(...)");
}

}} // namespace
