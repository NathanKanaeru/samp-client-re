#pragma once
// String utils (BasenameOf FUN_10062410: trim trailing space, cut at /,\, etc.)
#include <string>
namespace samp { namespace util {
std::string BasenameOf(const std::string& path);
std::string TrimRight(const std::string& s);
}} // namespace
