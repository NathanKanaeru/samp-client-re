#pragma once
// Input. Observed: GetAsyncKeyState polling FUN_10005f40 (Shift/Left/Right +
// debounce) + FUN_100751b0 (RMB/F1/F8/F9), WndProc hook FUN_10061970 ->
// proc 0x100610d0 (body UNKNOWN, not_in_procedure), cmd table FUN_10068b50
// via FUN_100691b0 (fpslimit/pagesize/fontsize/nametagstatus/timestamp/
// headmove/hudscalefix/audiomsg/ctd/dl/...), fontweight/fontface getters.
namespace samp { namespace input {
void PollChatKeys();   // FUN_10005f40
void Frame();          // FUN_100751b0
bool HookWndProc(void* hwnd); // FUN_10061970 (SetWindowLongA -4)
void RegisterCommands();      // FUN_10068b50
}} // namespace
