#pragma once
// Screenshot. Observed: FUN_10061cf0 (loop 0..999 FindFirstFileA),
// FUN_10075040 (D3DXSaveSurfaceToFileA), strings \screens\sa-mp-%03i.png,
// "Screenshot Taken - ...", "Unable to save screenshot.".
#include <string>
namespace samp { namespace gui {
int FindFreeShotSlot(const std::string& dir); // -1 if none
bool TakeScreenshot(const std::string& dir);
}} // namespace
