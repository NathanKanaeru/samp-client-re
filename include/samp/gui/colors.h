#pragma once
// Color-embed tags. Exact rule ported from samp-source client/runutil.cpp
// GetColorFromStringEmbed[W] (verified by read): `{` + 6 hex chars + `}`.
// Returns -1 when absent. Applied to chat/dialog/gametext rendering.
#include <cstdint>
#include <string>

namespace samp { namespace gui {

inline constexpr int kNoColor = -1;

// Parses a tag at s (needs >=8 chars). Returns RGB (no alpha) or kNoColor.
int ParseColorTag(const char* s);
// Removes all valid 8-char tags in place semantics (returns stripped copy).
std::string StripColorTags(const std::string& s);
bool IsHexChar(char c);

}} // namespace
