#pragma once
// GTA version gate. Observed: FUN_100aa300 (poll DAT_00747483), FUN_100a06d0.
#include <cstdint>
namespace samp { namespace core {
enum class GtaVersion { Unknown = 0, US10 = 1, EU10 = 2 };
GtaVersion DetectGtaVersionBlocking(); // polls ~6000x Sleep(10); false=>caller shows MessageBox+Exit
bool WaitForGtaState(uint32_t want, int polls); // polls a state dword (default gta::kStateDword==7)
}} // namespace
