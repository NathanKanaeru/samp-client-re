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
  // Rebuilds the sorted view from parallel arrays (lengths must match).
  // Returns false on length mismatch (view unchanged).
  bool UpdateList(const std::string* names, const int* scores, const int* pings, size_t n);
  const std::vector<Row>& rows() const { return rows_; }
private:
  bool visible_ = false;
  std::vector<Row> rows_;
};
Scoreboard& GetScoreboard();
}} // namespace
