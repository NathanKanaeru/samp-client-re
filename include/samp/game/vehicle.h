#pragma once
// Vehicle create/delete. Observed: FUN_1001eb40, FUN_100a0090 (new 0xB8),
// FUN_100b7f00 ctor, warn "Warning: vehicle %u was not deleted".
namespace samp { namespace game {
bool CreateVehicle(int id, int model, float x, float y, float z, float rot);
void DeleteVehicle(int id);
// Remote playback blend (PACKET200 doc constants): deadband 0.05/axis,
// snap |X|>8/|Y|>8 (Z 0.5, 2.0 if airOrBoat), correction error*0.06 applied
// when >0.01. Pure math, unit-tested.
struct RemoteFrame { float pos[3]; bool airOrBoat; };
void BlendVehiclePos(const float cur[3], const RemoteFrame& tgt, float out[3]);
}} // namespace
