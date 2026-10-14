#pragma once
// Player spawn / class. Observed: FUN_100109d0 (spawn packet),
// FUN_10016c50, FUN_10068440 (/savepositions -> AddPlayerClass fprintf).
namespace samp { namespace game {
bool SpawnPlayer(int id, int skin, float x, float y, float z, float rot);
void WriteAddPlayerClass(int id, float x, float y, float z, float rot, const char* name);
}} // namespace
