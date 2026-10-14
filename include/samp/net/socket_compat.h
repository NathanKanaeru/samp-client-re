#pragma once
// IPv4 socket primitives. Behavior ported from libsamp SPEC-NET-003 +
// ipv4_socket_compat.* (MIT), rewritten clean-room on Winsock2.
// recvfrom tri-state: 1 = packet, 0 = would-block, -1 = hard error.
#include <cstdint>
#include <cstddef>

namespace samp { namespace net {

inline constexpr unsigned kIpv4StrMax = 16u; // "xxx.xxx.xxx.xxx\0"
using Socket = uintptr_t;
inline constexpr Socket kInvalidSocket = (Socket)(~0u);

// 0 ok / -1 fail. Binds UDP, best-effort SO_REUSEADDR + RCVBUF 0x40000 +
// SNDBUF 0x4000 + BROADCAST; optional nonblocking; bind INADDR_ANY (or bindIp).
int UdpBind(uint16_t port, const char* bindIp, bool nonblocking, Socket* out);
void CloseSocket(Socket s);
// Returns bytes sent (<0 on error, *outError = WSA/errno). Retries zero-send
// up to 16 attempts. ipv4Be = big-endian address.
long UdpSendTo(Socket s, const void* data, size_t len, uint32_t ipv4Be,
               uint16_t port, int* outError);
// Tri-state described above. outLen/outAddrBe/outPort set on 1.
int UdpRecvFrom(Socket s, void* buf, size_t cap, size_t* outLen,
                uint32_t* outAddrBe, uint16_t* outPort);
// Local port of a bound socket (host order). 0 ok / -1 fail.
int GetsockPort(Socket s, uint16_t* out);
// First gethostbyname record. 0 ok / -1 fail.
int ResolveFirst(const char* host, uint32_t* outBe);
// Blocking TCP connect (literal-first, then resolve). Returns socket or invalid.
Socket TcpConnect(const char* host, uint16_t port);
// Listen on INADDR_ANY (backlog 0 -> 1). Returns socket or invalid.
Socket TcpListen(uint16_t port, int backlog);

bool IsWouldBlock(int errCode);
int LastNetError();

}} // namespace
