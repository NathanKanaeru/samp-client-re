// Query protocol. See query.h.
#include "samp/net/query.h"
#include <cstring>

namespace samp { namespace net {

static void PutU16(std::vector<uint8_t>& o, uint16_t v) {
  o.push_back((uint8_t)v); o.push_back((uint8_t)(v >> 8));
}

void BuildQuery(char opcode, uint32_t ipBe, uint16_t port, std::vector<uint8_t>& out) {
  out.clear();
  out.insert(out.end(), {'S', 'A', 'M', 'P'});
  // NOTE: original sends ip octets in network order as raw bytes.
  out.push_back((uint8_t)(ipBe >> 24)); out.push_back((uint8_t)(ipBe >> 16));
  out.push_back((uint8_t)(ipBe >> 8)); out.push_back((uint8_t)ipBe);
  out.push_back((uint8_t)port); out.push_back((uint8_t)(port >> 8));
  out.push_back((uint8_t)opcode);
}

void BuildRcon(const std::string& cmd, uint32_t ipBe, uint16_t port, std::vector<uint8_t>& out) {
  BuildQuery('c', ipBe, port, out);
  PutU16(out, (uint16_t)cmd.size());
  out.insert(out.end(), cmd.begin(), cmd.end());
}

struct Reader {
  const uint8_t* p; size_t n;
  bool take(void* dst, size_t k) {
    if (n < k) return false;
    memcpy(dst, p, k); p += k; n -= k; return true;
  }
  bool u8(uint8_t& v) { return take(&v, 1); }
  bool u16le(uint16_t& v) {
    uint8_t b[2]; if (!take(b, 2)) return false;
    v = (uint16_t)(b[0] | (b[1] << 8)); return true;
  }
  bool u32le(uint32_t& v) {
    uint8_t b[4]; if (!take(b, 4)) return false;
    v = b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
    return true;
  }
  bool str(std::string& s, size_t cap) {
    uint32_t len = 0;
    if (!u32le(len) || len > cap || len > n) return false;
    s.assign((const char*)p, len); p += len; n -= len; return true;
  }
};

static bool CheckHead(Reader& r, char opcode) {
  uint8_t magic[4];
  uint8_t op = 0;
  return r.take(magic, 4) && memcmp(magic, "SAMP", 4) == 0 && r.u8(op) && op == (uint8_t)opcode;
}

bool ParseInfoReply(const uint8_t* data, size_t len, QueryInfo& out) {
  Reader r{data, len};
  uint8_t locked = 0;
  if (!CheckHead(r, 'i') || !r.u8(locked) || !r.u16le(out.players) || !r.u16le(out.maxPlayers))
    return false;
  out.locked = locked != 0;
  return r.str(out.hostname, 256) && r.str(out.gamemode, 256) && r.str(out.language, 256);
}

bool ParseRulesReply(const uint8_t* data, size_t len, std::vector<QueryRule>& out) {
  Reader r{data, len};
  uint16_t count = 0;
  if (!CheckHead(r, 'r') || !r.u16le(count)) return false;
  out.clear();
  for (uint16_t i = 0; i < count; ++i) {
    uint8_t kl = 0, vl = 0;
    if (!r.u8(kl) || kl > r.n) return false;
    std::string k((const char*)r.p, kl); r.p += kl; r.n -= kl;
    if (!r.u8(vl) || vl > r.n) return false;
    std::string v((const char*)r.p, vl); r.p += vl; r.n -= vl;
    out.push_back({k, v});
  }
  return true;
}

bool ParseDetailReply(const uint8_t* data, size_t len, std::vector<QueryPlayer>& out) {
  Reader r{data, len};
  uint16_t count = 0;
  if (!CheckHead(r, 'd') || !r.u16le(count)) return false;
  out.clear();
  for (uint16_t i = 0; i < count; ++i) {
    QueryPlayer pl;
    uint8_t nl = 0;
    if (!r.u8(pl.id) || !r.u8(nl) || nl > r.n) return false;
    pl.name.assign((const char*)r.p, nl); r.p += nl; r.n -= nl;
    if (!r.u32le(pl.score) || !r.u32le(pl.ping)) return false;
    out.push_back(pl);
  }
  return true;
}

}} // namespace
