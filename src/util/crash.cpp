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

std::string FormatCrashFull(const CrashRegs& r, const char* version,
                            const uint32_t* stack, size_t n) {
  char b[4096];
  int o = snprintf(b, sizeof b,
                   "SA-MP %s\r\nException At Address: 0x%08X\r\nBase: 0x%08X\r\n\r\nRegisters:\r\n"
                   "EAX: 0x%08X\tEBX: 0x%08X\tECX: 0x%08X\tEDX: 0x%08X\r\n"
                   "ESI: 0x%08X\tEDI: 0x%08X\tEBP: 0x%08X\tESP: 0x%08X\r\n"
                   "EFLAGS: 0x%08X\r\n\r\nStack:\r\n",
                   version ? version : "0.3.DL-R1", r.eip, r.base,
                   r.eax, r.ebx, r.ecx, r.edx, r.esi, r.edi, r.ebp, r.esp, r.eflags);
  for (size_t i = 0; i < n && o > 0 && (size_t)o < sizeof b - 24; ++i)
    o += snprintf(b + o, sizeof b - (size_t)o, "+0x%02zX: 0x%08X\r\n", i * 4u, stack[i]);
  return b;
}

}} // namespace
