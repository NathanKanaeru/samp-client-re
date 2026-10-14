// Dialog/scoreboard/textdraw/fonts/screenshot. Observed addrs in headers.
#include "samp/gui/dialog.h"
#include "samp/gui/scoreboard.h"
#include "samp/gui/textdraw.h"
#include "samp/gui/fonts.h"
#include "samp/gui/screenshot.h"
#include <cstdio>
#include <windows.h>

namespace samp { namespace gui {

static Dialog g_dlg;
static Scoreboard g_sb;
static TextDrawPool g_td;
Dialog& GetDialog() { return g_dlg; }
Scoreboard& GetScoreboard() { return g_sb; }
TextDrawPool& TextDraws() { return g_td; }

bool Dialog::InitFonts(void*) { return true; } // 5x D3DXCreateFontA + sprite
void Dialog::ShowHelp() { // FUN_1006b570 text @0x1011c498
  printf("SA-MP Keys: F1 help, Tab scoreboard, F5 stats, F7 chat, F8 shot, F9 death, T/F6 talk\n");
}
void Dialog::Render() {}
bool Scoreboard::Init(void*) { return true; }
void Scoreboard::Render() {}
bool TextDrawPool::Init(void*) { return true; }
bool TextDrawPool::SetText(uint32_t id, const char* t, uint32_t size) {
  (void)id; (void)t;
  if (size > 0x1000) { printf("Warning: ignoring large TextDraw size=%u\n", size); return false; }
  return true;
}

int FontSizeFor(int base, int delta) { return base + delta * 2; } // FUN_100c6950
int FontWeightFor(int cfg) { return cfg == 0 ? 700 : (cfg == 1 ? 400 : 700); } // FUN_100c6a00
const char* FontFaceFor(const char* cfg) { return (cfg && *cfg) ? cfg : "Arial"; } // FUN_100c6a30
bool InstallGameFonts() { return true; } // AddFontResourceA x2 in DllMain

int FindFreeShotSlot(const std::string& dir) { // FUN_10061cf0 0..999
  for (int i = 0; i < 1000; ++i) {
    char p[MAX_PATH]; snprintf(p, sizeof p, "%s\\screens\\sa-mp-%03i.png", dir.c_str(), i);
    DWORD a = GetFileAttributesA(p);
    if (a == INVALID_FILE_ATTRIBUTES) return i;
  }
  return -1;
}
bool TakeScreenshot(const std::string& dir) { // FUN_10075040
  int s = FindFreeShotSlot(dir);
  if (s < 0) { printf("Unable to save screenshot.\n"); return false; }
  printf("Screenshot Taken - sa-mp-%03i.png\n", s);
  return true;
}

}} // namespace
