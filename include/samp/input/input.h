#pragma once
// Input. Observed: GetAsyncKeyState polling FUN_10005f40 (Shift/Left/Right +
// debounce) + FUN_100751b0 (RMB/F1/F8/F9), WndProc hook FUN_10061970 ->
// proc 0x100610d0 (body UNKNOWN, not_in_procedure), cmd table FUN_10068b50
// via FUN_100691b0 (fpslimit/pagesize/fontsize/nametagstatus/timestamp/
// headmove/hudscalefix/audiomsg/ctd/dl/...), fontweight/fontface getters.
// Reference (subclass.cpp): SubclassGameWindow sets "GTA:SA:MP" + GWL_WNDPROC;
// NewWndProc order: ACTIVATEAPP cursor, CmdWindow MsgProc, GameUI MsgProc,
// TAB scoreboard, SYSKEYUP-RETURN windowed toggle, key/char/mouse-wheel,
// SETCURSOR null, KILLFOCUS hide. Keys: F6 cmd, F7 chat, F8 screenshot flag,
// F9 death toggle, PGUP/PGDN scroll, UP/DOWN recall.
// Reference commands (cmdprocs.cpp:1546-1582): quit,q,save,rs,rcon,mem,
// pagesize,timestamp,hudscalefix,headmove,disvehico,testdw,vsel,v,vehicle,
// player_skin,set_weather,set_time,interior,cmpstat,dl,onfoot_correct,
// incar_correct,inacc_multiplier,fpslimit,audiomsg (+ debug-only set).
namespace samp { namespace input {
void PollChatKeys();   // FUN_10005f40
void Frame();          // FUN_100751b0
bool HookWndProc(void* hwnd); // FUN_10061970 (SetWindowLongA -4)
void RegisterCommands();      // FUN_10068b50
}} // namespace
