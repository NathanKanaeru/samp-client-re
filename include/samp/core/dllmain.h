#pragma once
// DllMain + globals. Observed: FUN_100c6780 @0x100c6780.
#include <windows.h>

namespace samp { namespace core {

extern HMODULE g_hInstance;   // DAT_102ac9c8
extern int     g_validA;      // DAT_102abe78
extern int     g_validB;      // DAT_102abe7c
extern void*   g_pArchive;    // DAT_102ac9d4 (CArchive*, 0x935 B)
extern void*   g_pManager;    // DAT_102aca3c (CManager*, 0x142 B)
extern void  (*g_pIdleTick)();// DAT_102ac9cc (FUN_100c66b0)

void ValidateGame();        // FUN_100c5c90: GetCommandLineA, zero 0x144 dword, -c/-d/-h/-l/-n/-p/-z
void InstallFileHooks();    // FUN_10062720
void InstallCursorHook();   // FUN_100623f0
void RemoveHooks();         // FUN_10062810
void IdleTick_Callback();   // FUN_100c66b0
void InitThreadProc(void*); // LAB_100c5150 (label, boundary inferred)

BOOL DllMainEntry(HMODULE h, DWORD reason, LPVOID reserved);

}} // namespace
