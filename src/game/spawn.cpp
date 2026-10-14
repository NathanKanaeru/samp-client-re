// Spawn FSM. See spawn.h for provenance.
#include "samp/game/spawn.h"

namespace samp { namespace game {

bool SpawnManager::RequestClass(int idx) {
  if (classes_.empty() || idx < 0 || idx >= (int)classes_.size()) return false;
  if (state_ != SpawnState::Idle) return false;
  selected_ = idx;
  state_ = SpawnState::ClassRequested;
  return true;
}

void SpawnManager::OnClassSettled() {
  if (state_ == SpawnState::ClassRequested) state_ = SpawnState::ClassSettled;
}

bool SpawnManager::RequestSpawn() {
  if (state_ != SpawnState::ClassSettled) return false;
  state_ = SpawnState::WaitSpawnReply;
  return true;
}

bool SpawnManager::Spawn(const SpawnEvents& ev) {
  if (!hasInfo_ || state_ != SpawnState::WaitSpawnReply) return false;
  if (ev.hideSpawnScreen) ev.hideSpawnScreen();
  if (ev.cameraBehindPlayer) ev.cameraBehindPlayer();
  if (ev.hudOn) ev.hudOn();
  if (ev.restartAt) ev.restartAt(info_.x, info_.y, info_.z, info_.rot);
  if (ev.setSkin) ev.setSkin(info_.skin);
  // NOTE: weapon clear + damage reset are GTA calls (no callback modeled).
  if (ev.setTeam) ev.setTeam(info_.team);
  for (int i = 2; i >= 0; --i) // reference order 2,1,0, skip -1
    if (info_.weapons[i] != -1 && ev.giveWeapon) ev.giveWeapon(info_.weapons[i], info_.ammo[i]);
  state_ = SpawnState::Spawned;
  return true;
}

bool ClassNavigator::Next(uint32_t nowMs) {
  if (count_ <= 0) return false;
  if (moved_ && nowMs - lastTick_ < kDebounceMs) return false;
  cur_ = (cur_ + 1) % count_;
  lastTick_ = nowMs;
  moved_ = true;
  return true;
}

bool ClassNavigator::Prev(uint32_t nowMs) {
  if (count_ <= 0) return false;
  if (moved_ && nowMs - lastTick_ < kDebounceMs) return false;
  cur_ = (cur_ - 1 + count_) % count_;
  lastTick_ = nowMs;
  moved_ = true;
  return true;
}

}} // namespace
