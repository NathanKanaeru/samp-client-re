// WndProc router. See wndproc.h for provenance (subclass.cpp flow).
#include "samp/input/wndproc.h"

namespace samp { namespace input {

bool RouteMessage(WndState& st, const WndEvent& ev) {
  switch (ev.msg) {
    case Msg::ActivateApp:
      // Reference: ShowCursor(FALSE) when game activates (subclass.cpp:247-251).
      return false; // let GTA see activation; cursor handled by hook layer
    case Msg::SysKeyUp:
      if (ev.wparam == VK_RET_) {
        // Toggle windowed mode (subclass.cpp:287-289). Swallowed.
        return true;
      }
      return false;
    case Msg::KeyDown:
      if (ev.wparam == VK_ESC_) {
        // Swallowed while cmd/scoreboard owns focus (subclass.cpp).
        if (st.cmdActive || st.scoreVisible) return true;
        return false;
      }
      return false;
    case Msg::KeyUp: {
      int vk = (int)ev.wparam;
      switch (vk) {
        case 0x09: // TAB: scoreboard toggle (subclass.cpp:270-281)
          st.scoreVisible = !st.scoreVisible;
          return true;
        case VK_F6_: st.cmdActive = !st.cmdActive; st.chatMode = st.cmdActive; return true;
        case VK_F7_: st.chatMode = true; st.cmdActive = true; return true;
        case VK_F8_: st.takeScreenshot = true; return false; // Present consumes
        case VK_F9_: st.deathVisible = !st.deathVisible; return true;
        case VK_PGUP: st.chatScroll += 1; return true;
        case VK_PGDN: st.chatScroll -= 1; return true;
        case VK_UP_: st.recallDir = -1; return true;
        case VK_DOWN_: st.recallDir = +1; return true;
        case VK_ESC_:
          if (st.cmdActive) { st.cmdActive = false; st.chatMode = false; return true; }
          if (st.scoreVisible) { st.scoreVisible = false; return true; }
          return false;
        default: return false;
      }
    }
    case Msg::Char: {
      unsigned ch = ev.wparam;
      if (ch == VK_ESC_) {
        if (st.cmdActive) { st.cmdActive = false; st.chatMode = false; return true; }
        if (st.scoreVisible) { st.scoreVisible = false; return true; }
        return false;
      }
      if ((ch == '`' || ch == 't' || ch == 'T') && !st.cmdActive) {
        st.cmdActive = true; st.chatMode = true; return true;
      }
      if (ch == 0x0A && st.cmdActive) return true; // swallow LF in cmd
      return false;
    }
    case Msg::MouseWheel:
      if (st.cmdActive) { // chat scroll while cmd open (subclass.cpp)
        st.chatScroll += (int)ev.wparam >= 0 ? 1 : -1;
        return true;
      }
      return false;
    case Msg::SetCursor:
      return true; // SetCursor(NULL) once (subclass.cpp: SE TCURSOR)
    case Msg::KillFocus:
      st.scoreVisible = false;
      st.cmdActive = false;
      st.chatMode = false;
      return false;
    case Msg::LButtonDown:
      if (st.scoreVisible && !ev.lbuttonInScoreRect) {
        st.outsideScoreboard = true;
        st.scoreVisible = false;
        return true; // swallowed (subclass.cpp:349-357)
      }
      st.outsideScoreboard = false;
      return false;
  }
  return false;
}

}} // namespace
