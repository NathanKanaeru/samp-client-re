#pragma once
// SAA archive layout/hash/find/extract. Behavior ported from two open refs:
//  - libsamp reimpl/src/archive/saa.c (MIT) — exact layout math, Jenkins
//    lookup3 (init 0x9E3779B9/seed 0x12345678), XTEA-stateful decrypt.
//  - samp-source client/archive/ArchiveFS.* — same hash + lookup idea.
// Cross-check vs REA on samp.dll 0.3.DL-R1: XOR accept mask 0x83433 ==
// SAA_FILE_ID, virtual handles 0xFF000001..step 2.
// Signature (RSA-128) verification itself stays external: table decrypt and
// extract are pure transforms; callers must gate on their own signature check
// (TODO(rea): Win32 CryptoAPI path) before trusting payloads.
#include <cstdint>
#include <cstddef>

namespace samp { namespace core {

namespace saa {
inline constexpr uint32_t kFileId      = 0x00083433u;
inline constexpr uint32_t kFileVersion = 2u;
inline constexpr uint32_t kFakeWords   = 120u;
inline constexpr size_t   kHeaderBytes = 8u + 120u * 2u + 8u; // 256
inline constexpr size_t   kEntryCount  = 256u;
inline constexpr size_t   kEntryBytes  = 8u;
inline constexpr size_t   kTableBytes  = 2048u;
inline constexpr size_t   kBlockBytes  = 2048u;
inline constexpr uint32_t kObfuscateKey = 0xC103D3E7u;
inline constexpr uint32_t kTeaRounds    = 32u;
inline constexpr uint32_t kTeaDelta     = 0x9E3779B9u;
inline constexpr uint32_t kHashInit     = 0x9E3779B9u;
inline constexpr uint32_t kHashSeed     = 0x12345678u;
// Known vectors (libsamp test_saa.c): regression anchors.
inline constexpr uint32_t kHashMainScm   = 0xD83F24DDu;
inline constexpr uint32_t kHashScriptImg = 0x11A462D1u;
} // namespace saa

struct SaaLayout {
  size_t header_size = 0;      // 256
  size_t entry_offset = 0;
  size_t signature_offset = 0;
  size_t signature_size = 0;   // 128 on stock files
  uint8_t invalid_index = 0;
};

struct SaaFilePos {
  uint32_t hash = 0;
  uint32_t size = 0;
  size_t data_offset = 0;
  uint16_t entry_index = 0;
};

// Jenkins lookup3 over raw bytes (a=b=0x9E3779B9, c=seed 0x12345678).
uint32_t SaaHashBytes(const uint8_t* data, size_t len);
// Lowercase-ASCII filename hash; empty/overlong -> 0. Trailing bytes kept
// (no trim): "main.scm " must NOT equal kHashMainScm (original behavior).
uint32_t SaaHashFilename(const char* filename);
// Basename after last \ or /, then SaaHashFilename. Empty -> false.
bool SaaHashPath(const char* path, uint32_t* out);
// Layout math only (no crypto): validates id/version/sizes/alignment.
bool SaaParseLayout(const uint8_t* data, size_t size, SaaLayout* out);
// In-place XTEA-stateful decrypt of the 2048-byte entry table with the
// documented 16-byte table key (original-DLL bytes, see saa.cpp).
void SaaDecryptTable(uint8_t table[saa::kTableBytes]);
// Find file by hash in a DECRYPTED table (see SaaDecryptTable): linear hash
// scan, reject previous==index||size==0, walk previous-chain to
// invalid_index with cycle + decoy + bound checks. entryTotal = byte offset
// where the entry table starts (== layout.entry_offset).
bool SaaFind(const uint8_t* entryTable, uint8_t invalidIndex,
             size_t headerSize, size_t entryTotal,
             uint32_t hash, SaaFilePos* out);
// Extract (decrypt per-2048B-block, key = table slice at hash%(2048-16)):
// needs raw file bytes + DECRYPTED table. out must hold >= file.size.
bool SaaExtract(const uint8_t* fileData, const uint8_t* entryTable,
                const SaaFilePos& file, uint8_t* out);

}} // namespace
