// Host:port parser. Clean-room rewrite of the SPEC-NET-002 rules.
#include "samp/net/endpoint.h"
#include <cstring>
#include <cctype>

namespace samp { namespace net {

static bool CopyHost(const char* src, size_t n, char out[256]) {
  if (n == 0u || n >= 256u) return false;
  memcpy(out, src, n);
  out[n] = '\0';
  return true;
}

static bool ParsePort(const char* s, size_t n, uint16_t* out) {
  if (n == 0u || n > 5u) return false;
  unsigned v = 0;
  for (size_t i = 0; i < n; ++i) {
    if (!isdigit((unsigned char)s[i])) return false;
    v = v * 10u + (unsigned)(s[i] - '0');
    if (v > 65535u) return false;
  }
  *out = (uint16_t)v;
  return true;
}

int ParseHostPort(const char* input, uint16_t defaultPort, Endpoint* out) {
  if (!input || !out || input[0] == '\0') return -1;
  memset(out, 0, sizeof *out);
  out->port = defaultPort;
  size_t len = strlen(input);
  if (input[0] == '[') {
    const char* close = (const char*)memchr(input, ']', len);
    if (!close) return -1;
    if (!CopyHost(input + 1, (size_t)(close - input - 1), out->host)) return -1;
    const char* rest = close + 1;
    if (*rest == '\0') return 0;
    if (*rest != ':') return -1;
    if (!ParsePort(rest + 1, strlen(rest + 1), &out->port)) return -1;
    return 0;
  }
  int colons = 0;
  for (size_t i = 0; i < len; ++i) if (input[i] == ':') ++colons;
  if (colons == 0) {
    if (!CopyHost(input, len, out->host)) return -1;
    return 0;
  }
  if (colons == 1) {
    const char* c = strchr(input, ':');
    if (!CopyHost(input, (size_t)(c - input), out->host)) return -1;
    if (!ParsePort(c + 1, strlen(c + 1), &out->port)) return -1;
    return 0;
  }
  // >1 colon without brackets: bare IPv6 literal, default port.
  if (!CopyHost(input, len, out->host)) return -1;
  return 0;
}

}} // namespace
