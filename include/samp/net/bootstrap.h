#pragma once
// TCP bootstrap manager lifecycle. Behavior ported from libsamp SPEC-NET-004
// + tcp_bootstrap_manager.* (MIT): idempotent init, duplicate-start reject,
// ephemeral-port resolve, stop/destroy counters. RakNet leg stripped to a
// boolean (real adapter lives behind net/raknet_adapter.h, TODO(rea)).
#include <cstdint>
#include "samp/net/socket_compat.h"

namespace samp { namespace net {

struct BootstrapManager {
  bool initialized = false;
  bool running = false;
  Socket listenSocket = kInvalidSocket;
  uint16_t listenPort = 0;
  unsigned initCount = 0;
  unsigned startCount = 0;
  unsigned stopCount = 0;
};

// Idempotent init (second call -> 0). -1 on null.
int BootstrapInit(BootstrapManager* m);
// Reject null/uninit/duplicate. Binds listener, resolves ephemeral port.
int BootstrapStartListener(BootstrapManager* m, uint16_t port, int backlog);
// Blocking connect helper (delegates to TcpConnect).
Socket BootstrapConnect(BootstrapManager* m, const char* host, uint16_t port);
// Idempotent stop (second call -> 0). -1 when never initialized.
int BootstrapStop(BootstrapManager* m);
// stop + reset to zero-state. -1 on null, 0 when never initialized.
int BootstrapDestroy(BootstrapManager* m);

}} // namespace
