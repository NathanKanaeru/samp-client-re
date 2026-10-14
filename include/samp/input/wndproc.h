#pragma once
// WndProc router. Flow ported from samp-source client/subclass.cpp
// (SubclassGameWindow + NewWndProc + HandleKeyPress + HandleCharacterInput).
// Pure state machine over abstract events (no HWND): fully unit-tested.
// Observed counterpart: FUN_10061970 hook -> proc 0x100610d0 (body unknown,
// so this is reference-shaped; wire to the real hook on x86 later).
#include <string>

namespace samp { namespace input {

// Abstracted message ids (mirror WM_* values used by the reference).
enum class Msg : unsigned {
  ActivateApp = 0x001C, KeyDown = 0x0100, KeyUp = 0x0101,
  Char = 0x0102, SysKeyUp = 0x0105, MouseWheel = 0x020A,
  SetCursor = 0x0020, KillFocus = 0x0008, LButtonDown = 0x0201,
};

// Virtual-key subset used by HandleKeyPress.
enum : int {
  VK_F6_ = 0x75, VK_F7_ = 0x76, VK_F8_ = 0x77, VK_F9_ = 0x78,
  VK_PGUP = 0x21, VK_PGDN = 0x22, VK_ESC_ = 0x1B, VK_UP_ = 0x26, VK_DOWN_ = 0x28,
  VK_RET_ = 0x0D,
};

struct WndState {
  bool cmdActive = false;
  bool chatMode = false;      // cmd open in chat-input mode
  bool scoreVisible = false;
  bool deathVisible = true;
  bool outsideScoreboard = false;
  bool takeScreenshot = false; // F8 flag consumed by Present hook
  int chatScroll = 0;          // PGUP/PGDN accumulator
  int recallDir = 0;           // last UP/DOWN: -1 older, +1 newer, 0 none
};

struct WndEvent {
  Msg msg;
  unsigned wparam = 0; // vk / char / wheel delta
  bool lbuttonInScoreRect = true; // for LButtonDown hit-test (injected)
};

// Returns true when the message is swallowed (do NOT CallWindowProc).
// Mutates state per reference order: activate-cursor, cmd/score routing,
// TAB toggle, ESC handling, key/char/wheel/cursor/focus/click.
bool RouteMessage(WndState& st, const WndEvent& ev);

}} // namespace
