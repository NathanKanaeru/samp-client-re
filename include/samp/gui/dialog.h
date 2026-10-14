#pragma once
// Dialog state. Reference: samp-source client/dialog.h (Show id/style/
// caption[64]/text/buttons, Hide) + shared.h DIALOG_STYLE_* 0..5.
// Pure state + list/tab parsing here (render stays in gui layer); tested.
#include <cstdint>
#include <string>
#include <vector>

namespace samp { namespace gui {

enum class DialogStyle : int {
  MsgBox = 0, Input = 1, List = 2, Password = 3, TabList = 4, TabListHeaders = 5,
};

struct DialogState {
  bool visible = false;
  int id = -1;
  DialogStyle style = DialogStyle::MsgBox;
  std::string caption; // <= 64 (MAX_DIALOG_CAPTION)
  std::string text;
  std::string button1;
  std::string button2;
  // Parsed rows for List/TabList(_Headers): split \n; columns split \t.
  std::vector<std::vector<std::string>> rows;
  int selected = -1; // list selection
};

class Dialog {
public:
  bool InitFonts(void* device);
  void ShowHelp(); // F1 text
  void Render();
  // Full dialog API (dialog.h: Show/Hide). Returns false on bad style.
  bool ShowDialog(int id, int style, const std::string& caption,
                  const std::string& text, const std::string& b1,
                  const std::string& b2);
  void Hide();
  bool SelectRow(int idx); // list dialogs; false when out of range/hidden
  const DialogState& state() const { return st_; }
private:
  DialogState st_;
};
Dialog& GetDialog();
}} // namespace
