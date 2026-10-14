#pragma once
// Exception filter + crash dialog. Observed: LAB_10060430 (filter),
// LAB_10060330 (dialog proc), FUN_10060260 (HTTP report).
#include <windows.h>
#include <cstdint>

namespace samp { namespace core {

LONG WINAPI TopLevelExceptionFilter(EXCEPTION_POINTERS* info);
INT_PTR CALLBACK CrashDialogProc(HWND dlg, UINT msg, WPARAM wp, LPARAM lp);
void ReportCrash_HTTP(uint32_t faultAddr);
void InstallCrashHandler();

}} // namespace
