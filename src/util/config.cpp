// Config. Observed: %s\sa-mp.cfg/chatlog.txt/screens + keys.
// Reference (configfile.*): lowercase case-insensitive keys, 30 entries,
// 40-char keys, typed int/float/string getters.
#include "samp/util/config.h"
#include <map>
#include <cctype>

namespace samp { namespace util {

static std::map<std::string, std::string> g_cfg;

static std::string Lower(std::string s) {
  for (auto& c : s) c = (char)tolower((unsigned char)c);
  return s;
}

static std::string Trim(const std::string& s) {
  size_t a = s.find_first_not_of(" \t\r\n");
  if (a == std::string::npos) return "";
  size_t b = s.find_last_not_of(" \t\r\n");
  return s.substr(a, b - a + 1);
}

std::string ConfigPath(const std::string& d) { return d + "\\sa-mp.cfg"; }
std::string ChatlogPath(const std::string& d) { return d + "\\chatlog.txt"; }
std::string ScreensDir(const std::string& d) { return d + "\\screens"; }
std::string GetCfg(const std::string& k, const std::string& def) {
  auto it = g_cfg.find(Lower(k));
  return it == g_cfg.end() ? def : it->second;
}
void SetCfg(const std::string& k, const std::string& v) {
  std::string key = Lower(k).substr(0, kMaxConfigKey);
  if (g_cfg.find(key) == g_cfg.end() && (int)g_cfg.size() >= kMaxConfigEntries)
    return; // full (reference drops beyond MAX_CONFIG_ENTRIES)
  g_cfg[key] = v;
}
int GetCfgInt(const std::string& k, int def) {
  auto it = g_cfg.find(Lower(k));
  if (it == g_cfg.end()) return def;
  try { return std::stoi(it->second); } catch (...) { return def; }
}
void ParseCfgText(const std::string& text) {
  size_t pos = 0;
  while (pos <= text.size()) {
    size_t nl = text.find('\n', pos);
    std::string line = Trim(text.substr(pos, nl == std::string::npos ? nl : nl - pos));
    if (!line.empty() && line[0] != '#') {
      size_t eq = line.find('=');
      if (eq != std::string::npos) SetCfg(Trim(line.substr(0, eq)), Trim(line.substr(eq + 1)));
    }
    if (nl == std::string::npos) break;
    pos = nl + 1;
  }
}
void ClearCfg() { g_cfg.clear(); }

}} // namespace
