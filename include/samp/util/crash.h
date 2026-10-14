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
}} // namespace
