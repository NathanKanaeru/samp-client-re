// Dialog state. See dialog.h (samp-source dialog.h + shared.h styles).
#include "samp/gui/dialog.h"

namespace samp { namespace gui {

static void ParseRows(DialogState& st) {
  st.rows.clear();
  st.selected = -1;
  int s = (int)st.style;
  if (s != 2 && s != 4 && s != 5) return;
  size_t pos = 0;
  while (pos <= st.text.size()) {
    size_t nl = st.text.find('\n', pos);
    std::string line = st.text.substr(pos, nl == std::string::npos ? nl : nl - pos);
    std::vector<std::string> cols;
    size_t c = 0;
    while (c <= line.size()) {
      size_t tb = line.find('\t', c);
      cols.push_back(line.substr(c, tb == std::string::npos ? tb : tb - c));
      if (tb == std::string::npos) break;
      c = tb + 1;
    }
    st.rows.push_back(cols);
    if (nl == std::string::npos) break;
    pos = nl + 1;
  }
}

bool Dialog::ShowDialog(int id, int style, const std::string& caption,
                        const std::string& text, const std::string& b1,
                        const std::string& b2) {
  if (style < 0 || style > 5) return false;
  st_.visible = true;
  st_.id = id;
  st_.style = (DialogStyle)style;
  st_.caption = caption.substr(0, 64); // MAX_DIALOG_CAPTION
  st_.text = text;
  st_.button1 = b1;
  st_.button2 = b2;
  ParseRows(st_);
  return true;
}

void Dialog::Hide() {
  st_.visible = false;
  st_.id = -1;
  st_.selected = -1;
}

bool Dialog::SelectRow(int idx) {
  if (!st_.visible) return false;
  int s = (int)st_.style;
  if (s != 2 && s != 4 && s != 5) return false;
  if (idx < 0 || idx >= (int)st_.rows.size()) return false;
  st_.selected = idx;
  return true;
}

}} // namespace
