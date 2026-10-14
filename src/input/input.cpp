// Input. Observed: FUN_10005f40, FUN_100751b0, FUN_10061970/0x100610d0,
// FUN_10068b50 table. No DX SDK needed.
#include "samp/input/input.h"
#include <windows.h>
#include <cstdio>

namespace samp { namespace input {

void PollChatKeys() { // FUN_10005f40: Shift 0x10, Left 0x25, Right 0x27 + debounce
  (void)GetAsyncKeyState(VK_SHIFT);
}

void Frame() { // FUN_100751b0: RMB 0x02, F1 0x70 help, F8 0x77 shot, F9 0x79
  if (GetAsyncKeyState(VK_F1) & 1) printf("[help]\n");
}

bool HookWndProc(void* hwnd) { // FUN_10061970: SetClassLong + SetWindowLong(GWL_WNDPROC)
  if (!hwnd) return false;
  SetWindowTextA((HWND)hwnd, "GTA:SA:MP");
  // TODO(rea): real proc body 0x100610d0 is not_in_procedure (naked ASM).
  return true;
}

void RegisterCommands() { // FUN_10068b50 via FUN_100691b0
  // Reference full list (cmdprocs.cpp:1546-1582): quit,q,save,rs,rcon,mem,
  // pagesize,timestamp,hudscalefix,headmove,disvehico,testdw,vsel,v,vehicle,
  // player_skin,set_weather,set_time,interior,cmpstat,dl,onfoot_correct,
  // incar_correct,inacc_multiplier,fpslimit,audiomsg (+ _DEBUG-only set).
  // TODO(rea): bind each name to its handler proc.
}

}} // namespace
