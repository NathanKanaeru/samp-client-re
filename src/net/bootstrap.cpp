// Bootstrap lifecycle. Clean-room rewrite of SPEC-NET-004 semantics.
#include "samp/net/bootstrap.h"
#include <cstring>

namespace samp { namespace net {

int BootstrapInit(BootstrapManager* m) {
  if (!m) return -1;
  if (m->initialized) return 0;
  memset(m, 0, sizeof *m);
  m->listenSocket = kInvalidSocket;
  m->initialized = true;
  m->initCount = 1;
  return 0;
}

int BootstrapStartListener(BootstrapManager* m, uint16_t port, int backlog) {
  if (!m || !m->initialized || m->running) return -1;
  Socket s = TcpListen(port, backlog);
  if (s == kInvalidSocket) return -1;
  uint16_t actual = port;
  if (port == 0 && GetsockPort(s, &actual) != 0) { CloseSocket(s); return -1; }
  m->listenSocket = s;
  m->listenPort = actual;
  m->running = true;
  ++m->startCount;
  return 0;
}

Socket BootstrapConnect(BootstrapManager* m, const char* host, uint16_t port) {
  if (!m || !m->initialized || !host) return kInvalidSocket;
  return TcpConnect(host, port);
}

int BootstrapStop(BootstrapManager* m) {
  if (!m || !m->initialized) return -1;
  if (m->listenSocket != kInvalidSocket) CloseSocket(m->listenSocket);
  m->listenSocket = kInvalidSocket;
  m->listenPort = 0;
  m->running = false;
  ++m->stopCount;
  return 0;
}

int BootstrapDestroy(BootstrapManager* m) {
  if (!m) return -1;
  if (!m->initialized) return 0;
  BootstrapStop(m);
  memset(m, 0, sizeof *m);
  m->listenSocket = kInvalidSocket;
  return 0;
}

}} // namespace
