// ValidateGame. Observed: FUN_100c5c90 @0x100c5c90 + helpers FUN_100c5590/55e0.
// Zeroes 0x144 dwords at DAT_102abe78, parses -c/-d/-h/-l/-n/-p/-z.
#include "samp/core/dllmain.h"
#include <windows.h>
#include <cstring>

namespace samp { namespace core {

static void CopyToken(char* dst, size_t cap, const char*& p) {
  size_t i = 0;
  if (*p == '"') { // CopyQuotedCmdToken FUN_100c55e0
    ++p;
    while (*p && *p != '"' && i + 1 < cap) dst[i++] = *p++;
    if (*p == '"') ++p;
  } else {         // CopyCmdToken FUN_100c5590
    while (*p && *p != ' ' && *p != '-' && *p != '/' && i + 1 < cap) dst[i++] = *p++;
  }
  dst[i] = 0;
}

void ValidateGame() {
  g_validA = 0; g_validB = 0;
  static char s_z[260] = {}, s_h[260] = {}, s_p[64] = {}, s_n[64] = {}, s_l[260] = {};
  (void)s_z; (void)s_h; (void)s_p; (void)s_n; (void)s_l;
  const char* cmd = GetCommandLineA();
  const char* p = cmd;
  char tok[260];
  while (*p) {
    while (*p == ' ') ++p;
    if (!*p) break;
    if (*p == '-' || *p == '/') {
      ++p;
      char opt = *p++;
      while (*p == ' ') ++p;
      switch (opt) {
        case 'c': g_validA = 0; g_validB = 1; CopyToken(tok, sizeof tok, p); break;
        case 'd': g_validA = 1; g_validB = 0; CopyToken(tok, sizeof tok, p); break;
        case 'h': CopyToken(s_h, sizeof s_h, p); break;
        case 'l': CopyToken(s_l, sizeof s_l, p); break;
        case 'n': CopyToken(s_n, sizeof s_n, p); break;
        case 'p': CopyToken(s_p, sizeof s_p, p); break;
        case 'z': CopyToken(s_z, sizeof s_z, p); break;
        default:  CopyToken(tok, sizeof tok, p); break;
      }
    } else {
      CopyToken(tok, sizeof tok, p);
    }
  }
}

}} // namespace
