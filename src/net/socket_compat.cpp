// IPv4 socket primitives. Clean-room Winsock2 implementation of SPEC-NET-003.
#include "samp/net/socket_compat.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <cstring>

namespace samp { namespace net {

static SOCKET ToSock(Socket s) { return (SOCKET)s; }

bool IsWouldBlock(int e) { return e == WSAEWOULDBLOCK; }
int LastNetError() { return WSAGetLastError(); }

int UdpBind(uint16_t port, const char* bindIp, bool nonblocking, Socket* out) {
  if (!out) return -1;
  SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (s == INVALID_SOCKET) return -1;
  int one = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof one);
  int rcv = 0x40000, snd = 0x4000;
  setsockopt(s, SOL_SOCKET, SO_RCVBUF, (const char*)&rcv, sizeof rcv);
  setsockopt(s, SOL_SOCKET, SO_SNDBUF, (const char*)&snd, sizeof snd);
  setsockopt(s, SOL_SOCKET, SO_BROADCAST, (const char*)&one, sizeof one);
  if (nonblocking) {
    u_long nb = 1;
    if (ioctlsocket(s, FIONBIO, &nb) != 0) { closesocket(s); return -1; }
  }
  sockaddr_in a{};
  a.sin_family = AF_INET;
  a.sin_port = htons(port);
  a.sin_addr.s_addr = bindIp ? inet_addr(bindIp) : htonl(INADDR_ANY);
  if (bind(s, (sockaddr*)&a, sizeof a) != 0) { closesocket(s); return -1; }
  *out = (Socket)s;
  return 0;
}

void CloseSocket(Socket s) {
  if (s != kInvalidSocket) closesocket(ToSock(s));
}

long UdpSendTo(Socket s, const void* data, size_t len, uint32_t ipv4Be,
               uint16_t port, int* outError) {
  if (s == kInvalidSocket || !data || len == 0u || len > (size_t)INT32_MAX) return -1;
  sockaddr_in a{};
  a.sin_family = AF_INET;
  a.sin_port = htons(port);
  a.sin_addr.s_addr = ipv4Be;
  int attempts = 0, rc = 0;
  while (rc == 0 && attempts < 16) {
    rc = sendto(ToSock(s), (const char*)data, (int)len, 0, (sockaddr*)&a, sizeof a);
    if (rc == 0) ++attempts;
  }
  if (rc < 0 && outError) *outError = WSAGetLastError();
  return rc;
}

int UdpRecvFrom(Socket s, void* buf, size_t cap, size_t* outLen,
                uint32_t* outAddrBe, uint16_t* outPort) {
  if (s == kInvalidSocket || !buf || cap == 0u) return -1;
  sockaddr_in a{};
  int alen = sizeof a;
  int rc = recvfrom(ToSock(s), (char*)buf, (int)cap, 0, (sockaddr*)&a, &alen);
  if (rc < 0) {
    if (WSAGetLastError() == WSAEWOULDBLOCK) { if (outLen) *outLen = 0; return 0; }
    if (outLen) *outLen = 0;
    return -1;
  }
  if (outLen) *outLen = (size_t)rc;
  if (outAddrBe) *outAddrBe = a.sin_addr.s_addr;
  if (outPort) *outPort = ntohs(a.sin_port);
  return 1;
}

int GetsockPort(Socket s, uint16_t* out) {
  if (s == kInvalidSocket || !out) return -1;
  sockaddr_in a{};
  int alen = sizeof a;
  if (getsockname(ToSock(s), (sockaddr*)&a, &alen) != 0) return -1;
  *out = ntohs(a.sin_port);
  return 0;
}

int ResolveFirst(const char* host, uint32_t* outBe) {
  if (!host || !outBe) return -1;
  hostent* he = gethostbyname(host);
  if (!he || !he->h_addr_list || !he->h_addr_list[0]) return -1;
  memcpy(outBe, he->h_addr_list[0], 4u);
  return 0;
}

Socket TcpConnect(const char* host, uint16_t port) {
  if (!host) return kInvalidSocket;
  uint32_t be = inet_addr(host);
  if (be == INADDR_NONE) {
    if (ResolveFirst(host, &be) != 0) return kInvalidSocket;
  }
  SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (s == INVALID_SOCKET) return kInvalidSocket;
  sockaddr_in a{};
  a.sin_family = AF_INET;
  a.sin_port = htons(port);
  a.sin_addr.s_addr = be;
  if (connect(s, (sockaddr*)&a, sizeof a) != 0) { closesocket(s); return kInvalidSocket; }
  return (Socket)s;
}

Socket TcpListen(uint16_t port, int backlog) {
  SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (s == INVALID_SOCKET) return kInvalidSocket;
  int one = 1;
  setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&one, sizeof one);
  sockaddr_in a{};
  a.sin_family = AF_INET;
  a.sin_port = htons(port);
  a.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(s, (sockaddr*)&a, sizeof a) != 0) { closesocket(s); return kInvalidSocket; }
  if (listen(s, backlog == 0 ? 1 : backlog) != 0) { closesocket(s); return kInvalidSocket; }
  return (Socket)s;
}

}} // namespace
