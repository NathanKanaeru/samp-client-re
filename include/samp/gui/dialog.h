#pragma once
// Dialog. Observed: FUN_1006b320 (5x D3DXCreateFontA + sprite + calloc 0x186a1),
// FUN_100695f0 (SAMPAUX3 fonts), FUN_10071600 (D3DXCreateLine), FUN_1006b570 (F1 help).
namespace samp { namespace gui {
class Dialog {
public:
  bool InitFonts(void* device);
  void ShowHelp(); // "SA-MP Keys: F1..." @0x1011c498
  void Render();
};
Dialog& GetDialog();
}} // namespace
