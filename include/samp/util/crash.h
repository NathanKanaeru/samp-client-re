#pragma once
// Crash format + report. Observed: FUN_10060070 (sprintf "SA-MP %s Exception
// At Address..." + regs + stack + SCM/Game Version), FUN_10060260 (POST
// team.sa-mp.com/report_037.php?addr=0x%X&gta=sa via FUN_10002c70 to www.sa-mp.com).
#include <windows.h>
#include <cstdint>
#include <string>
namespace samp { namespace util {
std::string FormatCrash(EXCEPTION_POINTERS* info, const char* version);
void ReportCrash(uint32_t faultAddr);
// Full original-shape report (REA string @0x1011b610): version header,
// fault/base addresses, 8 registers + EFLAGS, stack words. Pure + tested.
struct CrashRegs {
  uint32_t eip = 0, base = 0;
  uint32_t eax = 0, ebx = 0, ecx = 0, edx = 0;
  uint32_t esi = 0, edi = 0, ebp = 0, esp = 0, eflags = 0;
};
std::string FormatCrashFull(const CrashRegs& r, const char* version,
                            const uint32_t* stack, size_t stackWords);
}} // namespace
