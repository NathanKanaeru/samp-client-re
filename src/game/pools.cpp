// Pools. Observed caps + ctor sizes (vehicle 0xB8, player 0x32D).
#include "samp/game/pools.h"

namespace samp { namespace game {

static VehiclePool g_vp;
static PlayerPool g_pp;
VehiclePool& Vehicles() { return g_vp; }
PlayerPool& Players() { return g_pp; }

bool VehiclePool::Create(int id, int, float x, float y, float z, float) {
  if (id < 0 || id >= kMaxVehicle) return false;
  slots_[id].active = 1;
  slots_[id].pos[0] = x; slots_[id].pos[1] = y; slots_[id].pos[2] = z;
  return true;
}
void VehiclePool::Delete(int id) { if (id >= 0 && id < kMaxVehicle) slots_[id].active = 0; }
VehicleEntry* VehiclePool::Get(int id) {
  if (id < 0 || id >= kMaxVehicle || !slots_[id].active) return nullptr;
  return &slots_[id];
}
int PlayerPool::Alloc() {
  for (int i = 0; i < kMaxPlayer; ++i) if (!slots_[i].used) { slots_[i].used = 1; return i; }
  return -1;
}
bool PlayerPool::Spawn(int id, int, float, float, float, float) {
  if (id < 0 || id >= kMaxPlayer) return false;
  slots_[id].used = 1; return true;
}
void PlayerPool::Remove(int id) { if (id >= 0 && id < kMaxPlayer) slots_[id].used = 0; }
PlayerEntry* PlayerPool::Get(int id) {
  if (id < 0 || id >= kMaxPlayer || !slots_[id].used) return nullptr;
  return &slots_[id];
}

}} // namespace
