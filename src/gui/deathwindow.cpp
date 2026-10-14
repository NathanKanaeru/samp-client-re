// Death window ring. See deathwindow.h.
#include "samp/gui/deathwindow.h"

namespace samp { namespace gui {

void DeathWindow::Add(const std::string& killer, const std::string& killee, int reason) {
  DeathEntry e{killer, killee, reason};
  if (count_ < kMaxDeathDisp) entries_[count_++] = e;
  else {
    for (int i = 0; i < kMaxDeathDisp - 1; ++i) entries_[i] = entries_[i + 1];
    entries_[kMaxDeathDisp - 1] = e;
  }
}

void DeathWindow::AddSpecial(const std::string& nick, int special) {
  Add(nick, "", special);
}

}} // namespace
