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

static float AbsF(float v) { return v < 0 ? -v : v; }

void BlendVehiclePos(const float cur[3], const RemoteFrame& tgt, float out[3]) {
  float snapZ = tgt.airOrBoat ? 2.0f : 0.5f;
  float err[3] = {tgt.pos[0] - cur[0], tgt.pos[1] - cur[1], tgt.pos[2] - cur[2]};
  if (AbsF(err[0]) > 8.0f || AbsF(err[1]) > 8.0f || AbsF(err[2]) > snapZ) {
    out[0] = tgt.pos[0]; out[1] = tgt.pos[1]; out[2] = tgt.pos[2];
    return;
  }
  for (int i = 0; i < 3; ++i) {
    float corr = 0.0f;
    if (AbsF(err[i]) > 0.05f) corr = err[i] * 0.06f; // deadband + gain
    out[i] = (AbsF(corr) > 0.01f) ? cur[i] + corr : cur[i];
  }
}

}} // namespace
