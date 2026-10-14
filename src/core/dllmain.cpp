// DllMain reimplementation. Observed: FUN_100c6780 @0x100c6780.
// ATTACH: ValidateGame -> SEH -> samp.saa -> fonts -> hooks -> manager -> thread.
// DETACH: RemoveHooks. CRT entry 0x100fdb60 wraps this via thunk 0x100c6930.
#include "samp/core/dllmain.h"
#include "samp/core/archive.h"
#include "samp/core/exception.h"
#include "samp/core/version.h"
#include "samp/game/game.h"
#include <process.h>

namespace samp { namespace core {

HMODULE g_hInstance = nullptr;
int     g_validA = 0;
int     g_validB = 0;
void*   g_pArchive = nullptr;
void*   g_pManager = nullptr;
void   (*g_pIdleTick)() = nullptr;

static unsigned __stdcall InitThreadWrap(void* p) { InitThreadProc(p); return 0; }

BOOL DllMainEntry(HMODULE h, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) {
    g_hInstance = h;
    ValidateGame();
    if (!g_validA && !g_validB) return TRUE; // observed early-out
    InstallCrashHandler();
    g_pIdleTick = IdleTick_Callback;
    std::string saa = ArchivePathNextToModule(h);
    auto* ar = new CArchive();
    g_pArchive = ar;
    if (!ar->Open(saa.c_str())) {
      // Original executes INT3 here. We fail gracefully instead.
      // __debugbreak();
      return FALSE;
    }
    // Original: AddFontResourceA("gtaweap3.ttf" / "sampaux3.ttf")
    AddFontResourceA("gtaweap3.ttf");
    AddFontResourceA("sampaux3.ttf");
    InstallFileHooks();
    InstallCursorHook();
    auto* g = new game::CGame();
    g_pManager = g;
    game::SetGame(g);
    _beginthreadex(nullptr, 0, InitThreadWrap, nullptr, 0, nullptr);
  } else if (reason == DLL_PROCESS_DETACH) {
    if (g_validA || g_validB) RemoveHooks();
  }
  return TRUE;
}

void IdleTick_Callback() {
  // FUN_100c66b0: pumps FUN_100c6630, checks DAT_102aca68, GetTickCount
  // exit-after-1000ms logic. Full tick wiring is TODO(rea); keep as heartbeat.
}

void InitThreadProc(void*) {
  // LAB_100c5150: if (manager) DetectGtaVersion(); wait state==7 (Sleep 5);
  // FlagInitDone(); InitGameSystems(manager); ExitThread(0).
  if (g_pManager) {
    if (DetectGtaVersionBlocking() == GtaVersion::Unknown) return;
  }
  WaitForGtaState(7, 100000);
  game::FlagInitDone();
  game::InitGameSystems(static_cast<game::CGame*>(g_pManager));
}

}} // namespace

extern "C" BOOL WINAPI DllMain(HINSTANCE h, DWORD r, LPVOID x) {
  return samp::core::DllMainEntry(h, r, x);
}
