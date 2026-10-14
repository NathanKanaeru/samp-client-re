#pragma once
// Config (%s\sa-mp.cfg). Observed keys: fontsize/fontweight/fontface/
// timestamp/pagesize/fpslimit/audiomsg/audioproxyoff/multicore + chatlog.txt.
#include <string>
namespace samp { namespace util {
std::string ConfigPath(const std::string& dir);
std::string ChatlogPath(const std::string& dir);
std::string ScreensDir(const std::string& dir);
std::string GetCfg(const std::string& key, const std::string& def = "");
void SetCfg(const std::string& key, const std::string& value);
int  GetCfgInt(const std::string& key, int def);
}} // namespace
