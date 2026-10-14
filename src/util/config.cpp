// Config. Observed: %s\sa-mp.cfg/chatlog.txt/screens + keys.
#include "samp/util/config.h"
#include <map>

namespace samp { namespace util {

static std::map<std::string, std::string> g_cfg;
static std::string g_dir = ".";

std::string ConfigPath(const std::string& d) { return d + "\\sa-mp.cfg"; }
std::string ChatlogPath(const std::string& d) { return d + "\\chatlog.txt"; }
std::string ScreensDir(const std::string& d) { return d + "\\screens"; }
std::string GetCfg(const std::string& k, const std::string& def) {
  auto it = g_cfg.find(k);
  return it == g_cfg.end() ? def : it->second;
}
void SetCfg(const std::string& k, const std::string& v) { g_cfg[k] = v; }
int GetCfgInt(const std::string& k, int def) {
  auto it = g_cfg.find(k);
  if (it == g_cfg.end()) return def;
  try { return std::stoi(it->second); } catch (...) { return def; }
}

}} // namespace
