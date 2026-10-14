// Player spawn. Observed: FUN_100109d0 -> FUN_10016c50 -> FUN_1009ff50,
// "Warning: Couldn't spawn player(%u)", /savepositions AddPlayerClass writer.
#include "samp/game/player.h"
#include "samp/game/pools.h"
#include <cstdio>

namespace samp { namespace game {

bool SpawnPlayer(int id, int skin, float x, float y, float z, float rot) {
  (void)skin; (void)rot;
  if (!Players().Spawn(id, skin, x, y, z, rot)) {
    printf("Warning: Couldn't spawn player(%u)\n", (unsigned)id);
    return false;
  }
  return true;
}

void WriteAddPlayerClass(int id, float x, float y, float z, float rot, const char* name) {
  printf("AddPlayerClass(%u,%.4f,%.4f,%.4f,%.4f,0,0,0,0,0,0); // %s\n",
         (unsigned)id, x, y, z, rot, name ? name : "");
}

}} // namespace
