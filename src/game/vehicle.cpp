// Vehicle. Observed: FUN_1001eb40 alloc, "Warning: vehicle %u was not deleted".
#include "samp/game/vehicle.h"
#include "samp/game/pools.h"
#include <cstdio>

namespace samp { namespace game {

bool CreateVehicle(int id, int model, float x, float y, float z, float rot) {
  return Vehicles().Create(id, model, x, y, z, rot);
}
void DeleteVehicle(int id) {
  if (!Vehicles().Get(id)) printf("Warning: vehicle %u was not deleted\n", (unsigned)id);
  Vehicles().Delete(id);
}

}} // namespace
