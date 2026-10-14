#pragma once
// Fonts. Observed: FUN_100c6a30 (fontface else Arial), FUN_100c6a00
// (fontweight 700/400/300), FUN_100c6950 (size=base+delta*2),
// AddFontResourceA gtaweap3/sampaux3 in DllMain, CDXUT wrapper 0x1008c170.
namespace samp { namespace gui {
int FontSizeFor(int baseRes, int delta);
int FontWeightFor(int cfg); // 0->700 else 400/700
const char* FontFaceFor(const char* cfg); // empty->Arial
bool InstallGameFonts(); // AddFontResourceA x2
}} // namespace
