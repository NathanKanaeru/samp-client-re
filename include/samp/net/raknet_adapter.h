#pragma once
// RakNet client adapter (opaque C ABI). Interface shape ported from libsamp
#include <cstdint>
// reimpl/include/sampdll/net/raknet_client_adapter.h (MIT): lifecycle,
// send_* verbs, drain_packets pump, statistics — WITHOUT the RakNet submodule
// (third_party/raknet-knogle, currently unavailable here).
// Status: adapter unavailable -> Available()==false, all ops fail safe.
// TODO(rea): vendor Knogle/RakNet + implement drain loop (sync 200/207/209/
// 210/211/203/206 + RPC 1..200 observer + autojoin ClientJoin/AuthKey).
namespace samp { namespace net {
using RakClient = void*;
bool RakAdapterAvailable();
RakClient RakAdapterCreate();
void RakAdapterDestroy(RakClient* c);
bool RakAdapterConnect(RakClient c, const char* host, uint16_t serverPort);
bool RakAdapterIsConnected(RakClient c);
int  RakAdapterPump(RakClient c, int maxPackets); // drain -> count or -1
void RakAdapterDisconnect(RakClient c);
}} // namespace
