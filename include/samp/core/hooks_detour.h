#pragma once
// Inline JMP-detour engine (Detours-style). Observed: FUN_10110960 (alloc hook),
// FUN_10110700 (unhook/restore, checks opcode 0xE9), FUN_101111e0 (ResolveImport).
// Reference (samp-source filehooks.cpp:370-428): Microsoft Detours
// DetourFindFunction+DetourFunction, hooked APIs = CreateFileA, ReadFile,
// GetFileSize, SetFilePointer, CloseHandle, GetFileType, GetModuleHandleA
// (kernel32) + ShowCursor (user32). Virtual handles 0xFF000001..0xFF000101
// step 2 (CUSTOM_HANDLE_BASE/LIMIT), MAX_OPEN_ARCH_FILES 50.
#include <windows.h>
#include <cstdint>

namespace samp { namespace core {

inline constexpr uint32_t kCustomHandleBase  = 0xFF000001u;
inline constexpr uint32_t kCustomHandleLimit = 0xFF000101u;
inline constexpr int kMaxOpenArchFiles = 50;

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
