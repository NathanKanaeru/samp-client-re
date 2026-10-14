#pragma once
// Pools. Observed caps: vehicle 2000, player 1004/1005 (0x3EC/0x3ED),
// object 2101 (0x835), actor 1000, ped-slot map 210 @DAT_1018E1D8.
#include <cstdint>

namespace samp { namespace game {

inline constexpr int kMaxVehicle = 2000;
inline constexpr int kMaxPlayer  = 1004;
inline constexpr int kMaxObject  = 2101;
inline constexpr int kMaxActor   = 1000;

struct VehicleEntry { void* cv = nullptr; void* gta = nullptr; int active = 0; float pos[3] = {}; };
struct PlayerEntry  { void* cp = nullptr; int used = 0; };

class VehiclePool {
public:
  bool Create(int id, int model, float x, float y, float z, float rot); // FUN_1001eb40/100a0090
  void Delete(int id);
  VehicleEntry* Get(int id);
private:
  VehicleEntry slots_[kMaxVehicle];
};

class PlayerPool {
public:
  int  Alloc();              // scan DAT_1018E1D8, FUN_1009ff50, obj 0x32D
  bool Spawn(int id, int skin, float x, float y, float z, float rot); // FUN_100109d0/10016c50
  void Remove(int id);
  PlayerEntry* Get(int id);
private:
  PlayerEntry slots_[kMaxPlayer];
};

VehiclePool& Vehicles();
PlayerPool& Players();

}} // namespace
