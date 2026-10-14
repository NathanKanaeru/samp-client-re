// GtaVersion gate. Observed: FUN_100aa300 polls DAT_00747483
// (0x89=US1.0 -> 1, 0xC8=EU -> 2, else Sleep(10) up to 6000), FUN_100a06d0
// shows MessageBox "I can't determine your GTA version..." + ExitProcess(1).
#include "samp/core/version.h"
#include "samp/common.h"
#include <windows.h>

namespace samp { namespace core {

GtaVersion DetectGtaVersionBlocking() {
#ifdef _WIN64
  // x64 smoke-test build has no GTA memory; report US10 so init can proceed.
  (void)0;
  return GtaVersion::US10;
#else
  volatile uint8_t* p = (volatile uint8_t*)(uintptr_t)gta::kVersionByte;
  for (int i = 0; i < 6000; ++i) {
    __try {
      uint8_t v = *p;
      if (v == 0x89) return GtaVersion::US10;
      if (v == 0xC8) return GtaVersion::EU10;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return GtaVersion::Unknown; }
    Sleep(10);
  }
  return GtaVersion::Unknown;
#endif
}

bool WaitForGtaState(uint32_t want, int polls) {
#ifdef _WIN64
  (void)want; (void)polls;
  return true;
#else
  volatile uint32_t* p = (volatile uint32_t*)(uintptr_t)samp::gta::kStateDword;
  for (int i = 0; i < polls; ++i) {
    __try { if (*p == want) return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    Sleep(5);
  }
  return false;
#endif
}

}} // namespace
