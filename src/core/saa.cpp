// SAA layout/hash/find. See include/samp/core/saa.h for provenance.
// Written clean-room from the documented algorithm (Jenkins lookup3 public
// domain + layout math from libsamp saa.c MIT / samp-source ArchiveFS).
#include "samp/core/saa.h"
#include <cstring>

namespace samp { namespace core {

static uint32_t LoadLE32(const uint8_t* p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
         ((uint32_t)p[3] << 24);
}

static void HashMix(uint32_t* a, uint32_t* b, uint32_t* c) {
  *a -= *b; *a -= *c; *a ^= *c >> 13u;
  *b -= *c; *b -= *a; *b ^= *a << 8u;
  *c -= *a; *c -= *b; *c ^= *b >> 13u;
  *a -= *b; *a -= *c; *a ^= *c >> 12u;
  *b -= *c; *b -= *a; *b ^= *a << 16u;
  *c -= *a; *c -= *b; *c ^= *b >> 5u;
  *a -= *b; *a -= *c; *a ^= *c >> 3u;
  *b -= *c; *b -= *a; *b ^= *a << 10u;
  *c -= *a; *c -= *b; *c ^= *b >> 15u;
}

uint32_t SaaHashBytes(const uint8_t* data, size_t len) {
  uint32_t a = saa::kHashInit, b = saa::kHashInit, c = saa::kHashSeed;
  size_t rem = len;
  while (rem >= 12u) {
    a += LoadLE32(data); b += LoadLE32(data + 4u); c += LoadLE32(data + 8u);
    HashMix(&a, &b, &c);
    data += 12u; rem -= 12u;
  }
  c += (uint32_t)len;
  switch (rem) {
    case 11: c += (uint32_t)data[10] << 24u; [[fallthrough]];
    case 10: c += (uint32_t)data[9] << 16u; [[fallthrough]];
    case 9:  c += (uint32_t)data[8] << 8u; [[fallthrough]];
    case 8:  b += (uint32_t)data[7] << 24u; [[fallthrough]];
    case 7:  b += (uint32_t)data[6] << 16u; [[fallthrough]];
    case 6:  b += (uint32_t)data[5] << 8u; [[fallthrough]];
    case 5:  b += data[4]; [[fallthrough]];
    case 4:  a += (uint32_t)data[3] << 24u; [[fallthrough]];
    case 3:  a += (uint32_t)data[2] << 16u; [[fallthrough]];
    case 2:  a += (uint32_t)data[1] << 8u; [[fallthrough]];
    case 1:  a += data[0]; break;
    default: break;
  }
  HashMix(&a, &b, &c);
  return c;
}

uint32_t SaaHashFilename(const char* filename) {
  if (!filename) return 0u;
  uint8_t lowered[260];
  size_t n = 0;
  while (filename[n] != '\0' && n < sizeof lowered) {
    uint8_t ch = (uint8_t)filename[n];
    lowered[n] = (ch >= 'A' && ch <= 'Z') ? (uint8_t)(ch + ('a' - 'A')) : ch;
    ++n;
  }
  if (filename[n] != '\0' || n == 0u) return 0u;
  return SaaHashBytes(lowered, n);
}

bool SaaHashPath(const char* path, uint32_t* out) {
  if (!path || !out || path[0] == '\0') return false;
  const char* end = path + strlen(path);
  const char* base = end;
  while (base > path && base[-1] != '\\' && base[-1] != '/') --base;
  size_t n = (size_t)(end - base);
  char name[260];
  if (n == 0u || n >= sizeof name) return false;
  memcpy(name, base, n);
  name[n] = '\0';
  *out = SaaHashFilename(name);
  return *out != 0u;
}

bool SaaParseLayout(const uint8_t* data, size_t size, SaaLayout* out) {
  if (!data || !out) return false;
  if (size < saa::kHeaderBytes + saa::kTableBytes + 8u) return false;
  uint32_t encoded = LoadLE32(data + saa::kHeaderBytes - 8u);
  uint32_t key     = LoadLE32(data + saa::kHeaderBytes - 4u);
  uint32_t decoded = encoded ^ key;
  if ((decoded & 0xFFFFFu) != saa::kFileId) return false;
  if (((decoded >> 20u) & 7u) != saa::kFileVersion) return false;
  uint32_t sigSize = (decoded >> 23u) & 0xFFu;
  if (sigSize == 0u) return false;
  if (sigSize > size - saa::kHeaderBytes - saa::kTableBytes) return false;
  size_t sigOff = size - sigSize;
  size_t entOff = sigOff - saa::kTableBytes;
  if (entOff < saa::kHeaderBytes) return false;
  if ((entOff - saa::kHeaderBytes) % saa::kBlockBytes != 0u) return false;
  out->header_size = saa::kHeaderBytes;
  out->entry_offset = entOff;
  out->signature_offset = sigOff;
  out->signature_size = sigSize;
  out->invalid_index = (uint8_t)((key >> 5u) & 0xFFu);
  return true;
}

static size_t RoundBlock(uint32_t size) {
  return ((size_t)size + (saa::kBlockBytes - 1u)) & ~(size_t)(saa::kBlockBytes - 1u);
}

static uint32_t Unobfuscate(uint32_t v) {
  v ^= saa::kObfuscateKey;
  return (v >> 19u) | (v << 13u);
}

bool SaaFind(const uint8_t* entryTable, uint8_t invalidIndex,
             size_t headerSize, size_t entryTotal,
             uint32_t hash, SaaFilePos* out) {
  if (!entryTable || !out) return false;
  auto decode = [&](uint16_t idx, uint32_t* h, uint8_t* prev, uint32_t* sz) {
    if (idx >= saa::kEntryCount) return false;
    const uint8_t* e = entryTable + (size_t)idx * saa::kEntryBytes;
    uint32_t hv = LoadLE32(e);
    uint32_t dc = Unobfuscate(LoadLE32(e + 4u)) ^ hv;
    *h = hv; *prev = (uint8_t)dc; *sz = dc >> 8u;
    return true;
  };
  uint16_t index = 0;
  uint32_t h = 0; uint8_t prev = 0; uint32_t fsize = 0;
  for (; index < saa::kEntryCount; ++index) {
    if (!decode(index, &h, &prev, &fsize)) return false;
    if (h == hash) break;
  }
  if (index == saa::kEntryCount || prev == (uint8_t)index || fsize == 0u)
    return false;
  // Walk previous-chain: each step adds round_block(prev_size) to the offset.
  uint8_t visited[256] = {};
  size_t dataOff = headerSize;
  while (prev != invalidIndex) {
    if (visited[prev]) return false;
    visited[prev] = 1u;
    uint32_t ph = 0, psz = 0; uint8_t np = 0;
    if (!decode(prev, &ph, &np, &psz) || np == prev || psz == 0u) return false;
    size_t rounded = RoundBlock(psz);
    if (rounded > entryTotal - dataOff) return false;
    dataOff += rounded;
    prev = np;
  }
  if (RoundBlock(fsize) > entryTotal - dataOff) return false;
  out->hash = hash;
  out->size = fsize;
  out->data_offset = dataOff;
  out->entry_index = index;
  return true;
}

// Table key: original-DLL 16 bytes (libsamp saa.c documents offsets
// samp.dll+0xE9A2C, XOR 0xAA). Pure data constant, no code copied.
static const uint8_t kTableKeyObf[16] = {
  0xB9u, 0xEAu, 0x40u, 0x0Au, 0xA3u, 0x1Fu, 0x01u, 0x23u,
  0xB0u, 0xEAu, 0x46u, 0x64u, 0x78u, 0xAFu, 0x50u, 0x80u,
};

static void TeaLoadKey(uint32_t key[4], const uint8_t bytes[16], uint8_t x) {
  uint8_t d[16];
  for (int i = 0; i < 16; ++i) d[i] = (uint8_t)(bytes[i] ^ x);
  for (int i = 0; i < 4; ++i)
    key[i] = (uint32_t)d[i * 4] | ((uint32_t)d[i * 4 + 1] << 8) |
             ((uint32_t)d[i * 4 + 2] << 16) | ((uint32_t)d[i * 4 + 3] << 24);
}

static void TeaDecryptStateful(uint8_t* data, size_t size, uint32_t key[4]) {
  auto store = [](uint8_t* p, uint32_t v) {
    p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24);
  };
  for (size_t off = 0; off + 8u <= size; off += 8u) {
    uint32_t v0 = LoadLE32(data + off), v1 = LoadLE32(data + off + 4u);
    uint32_t o0 = v0, o1 = v1;
    uint32_t sum = saa::kTeaDelta * saa::kTeaRounds;
    for (uint32_t r = 0; r < saa::kTeaRounds; ++r) {
      v1 -= (((v0 << 4u) ^ (v0 >> 5u)) + v0) ^ (sum + key[(sum >> 11u) & 3u]);
      sum -= saa::kTeaDelta;
      v0 -= (((v1 << 4u) ^ (v1 >> 5u)) + v1) ^ (sum + key[sum & 3u]);
    }
    store(data + off, v0); store(data + off + 4u, v1);
    key[0] ^= o0; key[1] ^= o1; key[2] ^= o0; key[3] ^= o1;
  }
}

void SaaDecryptTable(uint8_t table[saa::kTableBytes]) {
  uint32_t key[4];
  TeaLoadKey(key, kTableKeyObf, 0xAAu);
  TeaDecryptStateful(table, saa::kTableBytes, key);
}

bool SaaExtract(const uint8_t* fileData, const uint8_t* entryTable,
                const SaaFilePos& file, uint8_t* out) {
  if (!fileData || !entryTable || !out || file.size == 0u) return false;
  uint8_t keyBytes[16];
  size_t keyOff = file.hash % (saa::kTableBytes - 16u);
  memcpy(keyBytes, entryTable + keyOff, 16u);
  uint32_t key[4];
  TeaLoadKey(key, keyBytes, 0u);
  size_t written = 0, srcOff = file.data_offset;
  uint8_t block[2048];
  while (written < file.size) {
    size_t want = file.size - written;
    if (want > sizeof block) want = sizeof block;
    memcpy(block, fileData + srcOff, sizeof block);
    TeaDecryptStateful(block, sizeof block, key);
    memcpy(out + written, block, want);
    written += want;
    srcOff += sizeof block;
  }
  return true;
}

}} // namespace
