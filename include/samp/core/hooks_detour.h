#pragma once
// Inline JMP-detour engine (Detours-style). Observed: FUN_10110960 (alloc hook),
// FUN_10110700 (unhook/restore, checks opcode 0xE9), FUN_101111e0 (ResolveImport).
#include <windows.h>
#include <cstdint>

namespace samp { namespace core {

struct HookHandle { void* target = nullptr; void* detour = nullptr; void* trampoline = nullptr; };

void* ResolveImport(const char* dll, const char* name); // FUN_101111e0
bool  InstallDetour(void* target, void* detour, HookHandle* out); // FUN_10110960-ish
bool  RemoveDetour(HookHandle* h);                                // FUN_10110700-ish

// Virtual-handle helpers (FUN_10062000 / FUN_10062040).
bool IsSampHandle(HANDLE h);
int  FindSlotForHandle(HANDLE h);

void InstallFileHooksImpl();
void InstallCursorHookImpl();
void RemoveHooksImpl();

}} // namespace
