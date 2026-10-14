#pragma once
// Common types + constants. Clean-room reimplementation of SA-MP 0.3.DL-R1 client.
// REA evidence: samp.dll PE x86, base 0x10000000, .text 0x10001000-0x10117000,
// 6293 procedures, 4265 strings (binary_overview ev_1a669035...).
#include <cstdint>
#include <cstddef>
#include <string>

namespace samp {

inline constexpr const char* kVersion = "0.3.DL-R1";
inline constexpr const char* kVersionTag = "{FFFFFF}SA-MP {B9C9BF}0.3.DL-R1 {FFFFFF}Started";
inline constexpr uint32_t kImageBaseOrig = 0x10000000u;

// Original object sizes observed in DllMain 0x100c6780 (operator_new args).
inline constexpr size_t kArchiveObjectSize = 0x935;
inline constexpr size_t kManagerObjectSize = 0x142;

// Virtual samp handles: FUN_10062000 checks 0xFF000000 < h < 0xFF000101.
inline constexpr uint32_t kVirtualHandleBase = 0xFF000000u;
inline constexpr uint32_t kVirtualHandleMax  = 0xFF000101u;
inline constexpr bool IsVirtualHandle(uint32_t h) {
  return h > kVirtualHandleBase && h < kVirtualHandleMax;
}

// GTA:SA 1.0 US/EU absolute addresses referenced by patcher (observed DAT_00xxxxxx).
// Kept as constants so 32-bit target build patches the right places; on x64 they are
// only used by tests / documentation, never dereferenced.
namespace gta {
inline constexpr uint32_t kStateDword   = 0x00C8D4C0u; // waited until ==7 by InitThreadProc
inline constexpr uint32_t kHwndDword    = 0x00C97C1Cu;
inline constexpr uint32_t kReadyByte    = 0x00BA6831u;
inline constexpr uint32_t kVersionByte  = 0x00747483u; // 0x89=US1.0, 0xC8=EU (FUN_100aa300)
inline constexpr uint32_t kDevicePtrUS1  = 0x007F9D50u; // FUN_100a1190 branch DAT_102ac9b8==1
inline constexpr uint32_t kDevicePtrUS2  = 0x007F9D90u; // FUN_100a1190 branch ==2
} // namespace gta

} // namespace samp
