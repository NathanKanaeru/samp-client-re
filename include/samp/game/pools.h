#pragma once
// Pools. Two cap families (documented discrepancy):
//  - Net slots (authoritative for protocol): net/limits.h
//    (MAX_PLAYERS 200, VEHICLES 700, OBJECTS 255, ACTORS 1000, ...).
//  - GTA-side overlay guards observed in samp.dll FUN_10004710:
//    vehicle 2000, player 1004/1005 (0x3EC/0x3ED), object 2101, actor 1000.
// Game pools below use NET caps (protocol truth); overlay guards live in
// VehiclePool::Get/PlayerPool::Get bounds checks.
#include <cstdint>
#include "samp/net/limits.h"

namespace samp { namespace game {

using net::kMaxPlayers;
using net::kMaxVehicles;
using net::kMaxObjects;
using net::kMaxActors;
// GTA-side overlay guard caps observed in FUN_10004710 (not protocol caps).
inline constexpr int kOverlayMaxVehicle = 2000;
inline constexpr int kOverlayMaxPlayer  = 1004;
inline constexpr int kOverlayMaxObject  = 2101;

inline constexpr int kMaxVehicle = kMaxVehicles;
inline constexpr int kMaxPlayer  = kMaxPlayers;
inline constexpr int kMaxObject  = kMaxObjects;
inline constexpr int kMaxActor   = kMaxActors;

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
