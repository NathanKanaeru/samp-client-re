// Crash format/report. Observed: FUN_10060070 + FUN_10060260.
#include "samp/util/crash.h"
#include <cstdio>

namespace samp { namespace util {

std::string FormatCrash(EXCEPTION_POINTERS* info, const char* version) {
  char buf[1024];
  uint32_t addr = info && info->ExceptionRecord
      ? (uint32_t)(uintptr_t)info->ExceptionRecord->ExceptionAddress : 0;
  snprintf(buf, sizeof buf, "SA-MP %s\r\nException At Address: 0x%08X\r\n",
           version ? version : "0.3.DL-R1", addr);
  return buf;
}

void ReportCrash(uint32_t faultAddr) {
  // Original POSTs team.sa-mp.com/report_037.php?addr=0x%X&gta=sa to www.sa-mp.com.
  // Keep side-effect free in reimplementation (log only).
  char url[160];
  snprintf(url, sizeof url, "team.sa-mp.com/report_037.php?addr=0x%X&gta=sa", faultAddr);
  (void)url;
}

}} // namespace
