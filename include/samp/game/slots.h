#pragma once
// Slot pools. Caps from net/limits.h (MAX_GANG_ZONES 1024, labels 2048,
// pickups 400, menus 128, vehicles 700, players 200, actors 1000).
// Generic fixed pool + concrete payload structs (samp-source *pool.h shapes).
// Pure logic, unit-tested.
#include <cstdint>
#include <string>
#include "samp/net/limits.h"

namespace samp { namespace game {

template <typename T, int N>
class SlotPool {
public:
  static constexpr int kCap = N;
  int Alloc() {
    for (int i = 0; i < N; ++i)
      if (!used_[i]) { used_[i] = true; slots_[i] = T(); ++count_; return i; }
    return -1;
  }
  bool Free(int id) {
    if (id < 0 || id >= N || !used_[id]) return false;
    used_[id] = false; slots_[id] = T(); --count_; return true;
  }
  T* Get(int id) { return (id >= 0 && id < N && used_[id]) ? &slots_[id] : nullptr; }
  const T* Get(int id) const { return (id >= 0 && id < N && used_[id]) ? &slots_[id] : nullptr; }
  int Count() const { return count_; }
private:
  T slots_[N];
  bool used_[N] = {};
  int count_ = 0;
};

struct GangZone { float x1 = 0, y1 = 0, x2 = 0, y2 = 0; uint32_t color = 0, alt = 0; };
struct MapLabel {
  std::string text; uint32_t color = 0;
  float x = 0, y = 0, z = 0, dist = 0;
  int16_t player = -1, vehicle = -1;
};
struct Pickup { int model = 0, type = 0; float x = 0, y = 0, z = 0; };
struct Menu {
  std::string title;
  std::string items[12]; // MAX_MENU_ITEMS
  int itemCount = 0;
};

using GangZonePool = SlotPool<GangZone, net::kMaxGangZones>;
using LabelPool = SlotPool<MapLabel, net::kMaxLabels>;
using PickupPool = SlotPool<Pickup, net::kMaxPickups>;
using MenuPool = SlotPool<Menu, net::kMaxMenus>;

}} // namespace
