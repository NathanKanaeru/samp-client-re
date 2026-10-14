// RakNet adapter stub: unavailable without the Knogle/RakNet submodule.
#include "samp/net/raknet_adapter.h"

namespace samp { namespace net {

bool RakAdapterAvailable() { return false; }
RakClient RakAdapterCreate() { return nullptr; }
void RakAdapterDestroy(RakClient* c) { if (c) *c = nullptr; }
bool RakAdapterConnect(RakClient, const char*, uint16_t) { return false; }
bool RakAdapterIsConnected(RakClient) { return false; }
int RakAdapterPump(RakClient, int) { return -1; }
void RakAdapterDisconnect(RakClient) {}

}} // namespace
