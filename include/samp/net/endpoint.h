#pragma once
// Endpoint + host:port parsing. Behavior ported from libsamp
// SPEC-NET-002 + dual_stack.c:132-230 (MIT): identical accept/reject rules,
// rewritten clean-room. REA note: samp.dll uses plain "host"+port fields
// (FUN_10008630 "Connecting to %s:%d"); this parser is a strict superset
// (adds [ipv6]:port + bare-ipv6), safe for join flow.
#include <cstdint>

namespace samp { namespace net {

struct Endpoint {
  char host[256] = {};
  uint16_t port = 0;
};

// Parses hostname | hostname:port | ipv4 | ipv4:port | ipv6 | [ipv6]:port.
// Returns 0 on success (out->port = parsed or defaultPort), -1 on error with
// no partial output. Port must be decimal 0..65535.
int ParseHostPort(const char* input, uint16_t defaultPort, Endpoint* out);

}} // namespace
