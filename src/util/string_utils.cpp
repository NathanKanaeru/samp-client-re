// BasenameOf FUN_10062410: trim trailing space, cut at /,\.
#include "samp/util/string_utils.h"

namespace samp { namespace util {

std::string TrimRight(const std::string& s) {
  size_t n = s.size();
  while (n && (s[n - 1] == ' ' || s[n - 1] == '\t')) --n;
  return s.substr(0, n);
}

std::string BasenameOf(const std::string& path) {
  std::string t = TrimRight(path);
  auto p = t.find_last_of("/\\");
  return p == std::string::npos ? t : t.substr(p + 1);
}

}} // namespace
