#pragma once
// Common types + constants. Clean-room reimplementation of SA-MP 0.3.DL-R1 client.
// REA evidence: samp.dll PE x86, base 0x10000000, .text 0x10001000-0x10117000,
// 6293 procedures, 4265 strings (binary_overview ev_1a669035...).
#include <cstdint>
#include <cstddef>
#include <string>
#include "samp/game/gta_addrs.h"

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

// GTA:SA 1.0 US/EU absolute addresses — canonical map now lives in
// game/gta_addrs.h (ported from samp-source address.h). Aliases kept so
// existing code still compiles.
namespace gta {
inline constexpr uint32_t kStateDword   = samp::gta::kEntryState;
inline constexpr uint32_t kHwndDword    = samp::gta::kHwnd;
inline constexpr uint32_t kReadyByte    = samp::gta::kGameStarted;
inline constexpr uint32_t kVersionByte  = samp::gta::kBypassVidsUS;
inline constexpr uint32_t kDevicePtrUS1  = samp::gta::kRwGetD3D_US;
inline constexpr uint32_t kDevicePtrUS2  = samp::gta::kRwGetD3D_EU;
} // namespace gta

} // namespace samp
