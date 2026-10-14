#pragma once
// API name registry. Observed: FUN_100203f0 (RegisterApi trampoline to
// FUN_10020260), FUN_10034cd0 (RakPeer method names), FUN_10030db0
// (Subscribe/Unsubscribe), FUN_10036a00 (console SetPassword/Clear/GetPassword).
// Game RPC + RCON handlers = UNKNOWN (ID_RPC 0xBC not found; TODO(rea)).
#include <string>
#include <vector>

namespace samp { namespace net {

struct ApiEntry { int argc; std::string name; std::string signature; };
void RegisterApi(int argc, const char* name, const char* sig); // FUN_100203f0
const std::vector<ApiEntry>& ApiRegistry();
void RegisterRakPeerApi(); // FUN_10034cd0 list

}} // namespace
