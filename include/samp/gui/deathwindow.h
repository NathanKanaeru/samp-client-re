#pragma once
// Death window. Reference: samp-source client/deathwindow.h (MAX_DISP 5,
// ring entries + SPECIAL_ENTRY_CONNECT 200 / DISCONNECT 201).
// Pure state machine here (render stays in gui layer); unit-tested.
#include <cstdint>
#include <string>

namespace samp { namespace gui {

inline constexpr int kMaxDeathDisp = 5;
inline constexpr int kSpecialConnect = 200;
inline constexpr int kSpecialDisconnect = 201;

struct DeathEntry {
  std::string killer;
  std::string killee;
  int reason = 0; // weapon id, or kSpecialConnect/kSpecialDisconnect
};

class DeathWindow {
public:
  void Add(const std::string& killer, const std::string& killee, int reason);
  void AddSpecial(const std::string& nick, int special);
  int Count() const { return count_; }
  const DeathEntry& At(int i) const { return entries_[i]; } // 0 = oldest shown
  void Clear() { count_ = 0; }
private:
  DeathEntry entries_[kMaxDeathDisp];
  int count_ = 0;
};

}} // namespace
