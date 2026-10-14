#pragma once
// GTA memory patcher. Observed: FUN_100a5d80 (patch dword+memcpy via
// VirtualProtect), FUN_100a9d10, FUN_100aa0b0, FUN_100a5e30 (6 CALL + vtable),
// FUN_100a6020 (stage2), FUN_100a6d50/100a6f00/100a19f0, FUN_100aaad0 (NOP-er),
// FUN_100aa300 (version gate). No WriteProcessMemory import (observed absent).
#include <cstdint>
#include <cstddef>

namespace samp { namespace game {

bool PatchMemcpy(uintptr_t dst, const void* src, size_t n); // FUN_100a5d80
bool PatchNop(uintptr_t addr, size_t n);                     // FUN_100aaad0 chunks
bool PatchCall(uintptr_t addr, void* target);                // E8 rel32
bool PatchJmp(uintptr_t addr, void* target);                 // E9 rel32
void InstallStage1();  // FUN_100a5e30
void InstallStage2();  // FUN_100a6020
void InstallStreamPatches(); // FUN_100a6d50/6f00/19f0
void InstallAllPatches();    // orchestrator ~FUN_100a0720 part

}} // namespace
