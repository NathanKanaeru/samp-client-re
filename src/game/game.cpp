// CGame. Observed: FUN_1009fdc0 ctor (new(5)+new(8) sub-objects,
// zero DAT_1018E1D8[0xD2] + field +0x6E[0x35], +0x55=0,+0x59=1,+0x5D=0x5A).
#include "samp/game/game.h"

namespace samp { namespace game {

static CGame* g_game = nullptr;

CGame::CGame() {
  p0_ = (void*)0x1; p1_ = (void*)0x2; // placeholder sub-objects
  f_[0x55] = 0; f_[0x59] = 1; f_[0x5D] = 0x5A;
}

CGame* GetGame() { return g_game; }
void SetGame(CGame* g) { g_game = g; }
void FlagInitDone() {}        // FUN_1009db90: _DAT_1018dc38=1
void InitGameSystems(CGame*) {} // FUN_100a0720: 100aaad0/100a6020/100b2150/1009c980
void GameThreadProc(void*) {}

}} // namespace
