// Exception filter + crash dialog. Observed: LAB_10060430,
// LAB_10060330, FUN_10060260 (sprintf + POST report_037.php).
#include "samp/core/exception.h"
#include "samp/util/crash.h"
#include "samp/common.h"

namespace samp { namespace core {

LONG WINAPI TopLevelExceptionFilter(EXCEPTION_POINTERS* info) {
  uint32_t addr = info && info->ExceptionRecord
      ? (uint32_t)(uintptr_t)info->ExceptionRecord->ExceptionAddress : 0;
  ReportCrash_HTTP(addr);
  return EXCEPTION_EXECUTE_HANDLER; // observed return 1
}

INT_PTR CALLBACK CrashDialogProc(HWND dlg, UINT msg, WPARAM wp, LPARAM) {
  if (msg == WM_COMMAND) {
    if (LOWORD(wp) == IDOK || LOWORD(wp) == IDCANCEL) {
      EndDialog(dlg, LOWORD(wp));
      return TRUE;
    }
  }
  return FALSE;
}

void ReportCrash_HTTP(uint32_t faultAddr) {
  util::ReportCrash(faultAddr);
}

void InstallCrashHandler() {
  SetUnhandledExceptionFilter(TopLevelExceptionFilter);
}

}} // namespace
