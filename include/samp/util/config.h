#pragma once
// Config (%s\sa-mp.cfg). Observed keys: fontsize/fontweight/fontface/
// timestamp/pagesize/fpslimit/audiomsg/audioproxyoff/multicore + chatlog.txt.
// Reference (configfile.h): MAX_CONFIG_ENTRIES 30, MAX_CONFIG_KEY 40.
#include <string>
namespace samp { namespace util {
std::string ConfigPath(const std::string& dir);
std::string ChatlogPath(const std::string& dir);
std::string ScreensDir(const std::string& dir);
// Case-insensitive store (keys lowercased, like CConfigFile::AddKey with
// _strlwr + _stricmp find). Caps: 30 entries, 40-char keys (configfile.h).
inline constexpr int kMaxConfigEntries = 30;
inline constexpr int kMaxConfigKey = 40;
std::string GetCfg(const std::string& key, const std::string& def = "");
void SetCfg(const std::string& key, const std::string& value);
int  GetCfgInt(const std::string& key, int def);
// Parses "key=value" text (whitespace-trimmed, '#' comments); last wins.
void ParseCfgText(const std::string& text);
void ClearCfg();
}} // namespace
