// GTA patcher. Observed helpers use VirtualProtect (no WriteProcessMemory
// import). Addresses below are SA 1.0 US absolutes seen in FUN_100a5e30/a6020/
// aaad0. On x64 they are inert constants (never dereferenced).
#include "samp/game/gta_patches.h"
#include <windows.h>
#include <cstring>

namespace samp { namespace game {

bool PatchMemcpy(uintptr_t dst, const void* src, size_t n) {
#ifdef _WIN64
  (void)dst; (void)src; (void)n; return true;
#else
  DWORD old = 0;
  if (!VirtualProtect((void*)dst, n, PAGE_EXECUTE_READWRITE, &old)) return false;
  memcpy((void*)dst, src, n);
  DWORD t = 0; VirtualProtect((void*)dst, n, old, &t);
  return true;
#endif
}

bool PatchNop(uintptr_t addr, size_t n) {
#ifdef _WIN64
  (void)addr; (void)n; return true;
#else
  DWORD old = 0;
  if (!VirtualProtect((void*)addr, n, PAGE_EXECUTE_READWRITE, &old)) return false;
  memset((void*)addr, 0x90, n);
  DWORD t = 0; VirtualProtect((void*)addr, n, old, &t);
  return true;
#endif
}

bool PatchCall(uintptr_t addr, void* target) {
  int32_t rel = (int32_t)((uint8_t*)target - ((uint8_t*)addr + 5));
  uint8_t buf[5] = {0xE8, 0, 0, 0, 0};
  memcpy(buf + 1, &rel, 4);
  return PatchMemcpy(addr, buf, 5);
}

bool PatchJmp(uintptr_t addr, void* target) {
  int32_t rel = (int32_t)((uint8_t*)target - ((uint8_t*)addr + 5));
  uint8_t buf[5] = {0xE9, 0, 0, 0, 0};
  memcpy(buf + 1, &rel, 4);
  return PatchMemcpy(addr, buf, 5);
}

void InstallStage1() {
  // FUN_100a5e30: 6 CALL patches (0x0053EB13, 0x0058FC53, 0x0058FBBF,
  // 0x0053C104, 0x0053E981, 0x0053E930) + vtable ptrs 0x0086D1B0... -> hook.
  // TODO(rea): bind real hook targets (LAB_100a3b90 PUSHAD trampoline).
}
void InstallStage2() {
  // FUN_100a6020: E8 to 0x007330A2/0x005689FD/..., E9 to 0x004579C6/0x0073C252,
  // vtable captures 0x0086D190->LAB_100a26d0 etc.
}
void InstallStreamPatches() {} // FUN_100a6d50/6f00/19f0
void InstallAllPatches() { InstallStage1(); InstallStage2(); InstallStreamPatches(); }

}} // namespace
