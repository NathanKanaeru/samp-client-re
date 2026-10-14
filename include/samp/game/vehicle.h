#pragma once
// Vehicle create/delete. Observed: FUN_1001eb40, FUN_100a0090 (new 0xB8),
// FUN_100b7f00 ctor, warn "Warning: vehicle %u was not deleted".
namespace samp { namespace game {
bool CreateVehicle(int id, int model, float x, float y, float z, float rot);
void DeleteVehicle(int id);
}} // namespace
