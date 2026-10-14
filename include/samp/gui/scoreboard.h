#pragma once
// Scoreboard + death window. Observed: FUN_10069b90 (GTAWEAPON3 + sprite),
// LONGESTNICK measure, shared fonts with dialog.
namespace samp { namespace gui {
class Scoreboard {
public:
  bool Init(void* device);
  void Render();
  void SetVisible(bool v) { visible_ = v; }
private:
  bool visible_ = false;
};
Scoreboard& GetScoreboard();
}} // namespace
