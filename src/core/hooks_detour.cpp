// Detours-style hooks. Observed: InstallFileHooks 0x10062720 (6 file APIs),
// InstallCursorHook 0x100623f0 (ShowCursor), RemoveHooks 0x10062810,
// ResolveImport 0x101111e0, hook alloc 0x10110960, unhook 0x10110700
// (validates opcode 0xE9 JMP rel32), virtual handles 0xFF000000..0xFF000101.
#include "samp/core/hooks_detour.h"
#include "samp/common.h"
#include <windows.h>
#include <vector>

namespace samp { namespace core {

static std::vector<HookHandle> g_hooks;

void* ResolveImport(const char* dll, const char* name) {
  HMODULE m = GetModuleHandleA(dll);
  if (!m) m = LoadLibraryA(dll);
  if (!m) return nullptr;
  return (void*)GetProcAddress(m, name);
}

static bool WriteJmp(void* target, void* detour, void** tramp) {
  DWORD old = 0;
  if (!VirtualProtect(target, 5, PAGE_EXECUTE_READWRITE, &old)) return false;
  // Minimal trampoline: keep first 5 bytes + jump back. Full disasm is TODO(rea).
  uint8_t* t = (uint8_t*)VirtualAlloc(nullptr, 16, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if (!t) { VirtualProtect(target, 5, old, &old); return false; }
  memcpy(t, target, 5);
  t[5] = 0xE9;
  *(int32_t*)(t + 6) = (int32_t)((uint8_t*)target + 5 - (t + 10));
  *(uint8_t*)target = 0xE9;
  *(int32_t*)((uint8_t*)target + 1) =
      (int32_t)((uint8_t*)detour - ((uint8_t*)target + 5));
  VirtualProtect(target, 5, old, &old);
  if (tramp) *tramp = t;
  return true;
}

bool InstallDetour(void* target, void* detour, HookHandle* out) {
  HookHandle h{target, detour, nullptr};
  if (!WriteJmp(target, detour, &h.trampoline)) return false;
  if (out) *out = h;
  g_hooks.push_back(h);
  return true;
}

bool RemoveDetour(HookHandle* h) {
  if (!h || !h->target || !h->trampoline) return false;
  DWORD old = 0;
  if (!VirtualProtect(h->target, 5, PAGE_EXECUTE_READWRITE, &old)) return false;
  memcpy(h->target, h->trampoline, 5);
  VirtualProtect(h->target, 5, old, &old);
  VirtualFree(h->trampoline, 0, MEM_RELEASE);
  h->trampoline = nullptr;
  return true;
}

bool IsSampHandle(HANDLE h) {
  uint32_t v = (uint32_t)(uintptr_t)h;
  return samp::IsVirtualHandle(v);
}

int FindSlotForHandle(HANDLE) { return -1; } // table DAT_1016bf60/32 slots; TODO(rea)

static HANDLE WINAPI HookedCreateFileA(LPCSTR f, DWORD a, DWORD s, LPSECURITY_ATTRIBUTES sa,
                                       DWORD c, DWORD fl, HANDLE t) {
  using Fn = HANDLE(WINAPI*)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
  static Fn orig = nullptr;
  if (!orig) orig = (Fn)ResolveImport("kernel32.dll", "CreateFileA");
  // TODO(rea): consult CArchive for samp.saa members, return virtual handle.
  return orig(f, a, s, sa, c, fl, t);
}

void InstallFileHooksImpl() {
  HookHandle h;
  if (auto* t = ResolveImport("kernel32.dll", "CreateFileA"))
    InstallDetour(t, (void*)HookedCreateFileA, &h);
  // TODO(rea): ReadFile LAB_100620f0, GetFileSize FUN_10062270,
  // SetFilePointer FUN_100622c0, CloseHandle FUN_10062360, GetFileType FUN_100623a0.
}

void InstallCursorHookImpl() {
  // TODO(rea): user32.ShowCursor -> LAB_100623e0 (ECX==0?0:~0).
}

void RemoveHooksImpl() {
  for (auto& h : g_hooks) RemoveDetour(&h);
  g_hooks.clear();
}

void InstallFileHooks() { InstallFileHooksImpl(); }
void InstallCursorHook() { InstallCursorHookImpl(); }
void RemoveHooks() { RemoveHooksImpl(); }

}} // namespace
