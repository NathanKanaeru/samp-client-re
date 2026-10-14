#pragma once
// Spawn flow. Observed: FUN_100109d0 (spawn packet) -> FUN_10016c50 ->
// FUN_1009ff50, "Warning: Couldn't spawn player", class UI FUN_100c6ce0.
// Reference (localplayer.* + shared.h PLAYER_SPAWN_INFO): SetSpawnInfo,
// RequestClass(i)/RequestSpawn() RPCs 22/23, Spawn() order: hide spawn
// screen -> camera restore+behind -> HUD on -> controllable -> streaming
// refresh -> restart at pos/rot -> skin -> clear weapons -> team -> give
// weapons in 2,1,0 order (skip -1). Pure FSM here; GTA calls are callbacks.
#include <cstdint>
#include <string>
#include <vector>

namespace samp { namespace game {

struct SpawnInfo {
  uint8_t team = 0;
  int skin = 0;
  float x = 0, y = 0, z = 0, rot = 0;
  int weapons[3] = {-1, -1, -1};
  int ammo[3] = {0, 0, 0};
};

struct PlayerClass {
  int skin = 0;
  float x = 0, y = 0, z = 0, rot = 0;
};

enum class SpawnState : uint8_t {
  Idle, ClassRequested, ClassSettled, WaitSpawnReply, Spawned,
};

struct SpawnEvents {
  void (*hideSpawnScreen)() = nullptr;
  void (*cameraBehindPlayer)() = nullptr;
  void (*hudOn)() = nullptr;
  void (*restartAt)(float x, float y, float z, float rot) = nullptr;
  void (*setSkin)(int skin) = nullptr;
  void (*giveWeapon)(int weapon, int ammo) = nullptr;
  void (*setTeam)(uint8_t team) = nullptr;
};

class SpawnManager {
public:
  void AddClass(const PlayerClass& c) { classes_.push_back(c); }
  size_t ClassCount() const { return classes_.size(); }
  // Returns false when no classes / already past selection.
  bool RequestClass(int idx);
  int SelectedClass() const { return selected_; }
  void OnClassSettled();   // server accepted (RequestSpawn may follow)
  bool RequestSpawn();     // -> WaitSpawnReply (false unless ClassSettled)
  void SetSpawnInfo(const SpawnInfo& si) { info_ = si; hasInfo_ = true; }
  // Applies Spawn() order via callbacks. False when no info / wrong state.
  bool Spawn(const SpawnEvents& ev);
  SpawnState state() const { return state_; }
  const SpawnInfo& info() const { return info_; }
private:
  std::vector<PlayerClass> classes_;
  int selected_ = -1;
  SpawnInfo info_;
  bool hasInfo_ = false;
  SpawnState state_ = SpawnState::Idle;
};

}} // namespace
