#pragma once
// Scoreboard + death window. Observed: FUN_10069b90 (GTAWEAPON3 + sprite),
// LONGESTNICK measure, shared fonts with dialog.
#include <string>
#include <vector>
#include <algorithm>
namespace samp { namespace gui {
class Scoreboard {
public:
  struct Row { std::string name; int score = 0; int ping = 0; };
  bool Init(void* device);
  void Render();
  void SetVisible(bool v) { visible_ = v; }
  // Pure sort helper (score desc, name asc tiebreak); unit-tested.
  static void SortRows(std::vector<Row>& rows) {
    std::sort(rows.begin(), rows.end(), [](const Row& a, const Row& b) {
      if (a.score != b.score) return a.score > b.score;
      return a.name < b.name;
    });
  }
private:
  bool visible_ = false;
};
Scoreboard& GetScoreboard();
}} // namespace
