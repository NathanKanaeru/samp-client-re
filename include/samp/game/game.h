#pragma once
// CGame manager (0x142 bytes). Observed: FUN_1009fdc0 ctor, DAT_102aca3c,
// InitThreadProc LAB_100c5150 -> DetectGtaVersion 0x100a06d0 ->
// wait state==7 -> FlagInitDone 0x1009db90 -> InitGameSystems 0x100a0720.
#include <cstdint>

namespace samp { namespace game {

class CGame {
public:
  static constexpr size_t kSize = 0x142;
  CGame();
  void* p0() const { return p0_; }
  void* p1() const { return p1_; }
  uint8_t state55() const { return s55_; }
private:
  void* p0_ = nullptr;   // +0: operator_new(5) sub-object
  void* p1_ = nullptr;   // +1: operator_new(8) sub-object
  uint32_t pad2_ = 0;
  uint8_t  f_[0x142 - 12] = {};
  uint8_t  s55_ = 0;     // +0x55 = 0 observed
};

CGame* GetGame();        // DAT_102aca3c
void SetGame(CGame* g);
void GameThreadProc(void*); // LAB_100c5150
void FlagInitDone();        // FUN_1009db90
void InitGameSystems(CGame* g); // FUN_100a0720

}} // namespace
