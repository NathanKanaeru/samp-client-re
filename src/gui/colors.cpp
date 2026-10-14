// Color tags. See colors.h for provenance.
#include "samp/gui/colors.h"

namespace samp { namespace gui {

bool IsHexChar(char c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static int HexVal(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return c - 'a' + 10;
}

int ParseColorTag(const char* s) {
  if (!s || s[0] != '{') return kNoColor;
  for (int i = 1; i <= 6; ++i)
    if (!s[i] || !IsHexChar(s[i])) return kNoColor;
  if (s[7] != '}') return kNoColor;
  int v = 0;
  for (int i = 1; i <= 6; ++i) v = v * 16 + HexVal(s[i]);
  return v;
}

std::string StripColorTags(const std::string& s) {
  std::string out;
  out.reserve(s.size());
  for (size_t i = 0; i < s.size();) {
    if (s[i] == '{' && i + 8 <= s.size() && ParseColorTag(s.c_str() + i) != kNoColor)
      i += 8;
    else
      out.push_back(s[i++]);
  }
  return out;
}

}} // namespace
