#pragma once
// TextDraw pool. Observed: calloc 0x186a1 in FUN_1006b320, guard
// "?Warning: ignoring large TextDraw size=%u" @0x10118493 (no xref).
// Render fn exact = UNKNOWN (TODO(rea)).
#include <cstdint>
namespace samp { namespace gui {
inline constexpr size_t kTextDrawPoolAlloc = 0x186a1;
class TextDrawPool {
public:
  bool Init(void* device);
  bool SetText(uint32_t id, const char* text, uint32_t size);
};
TextDrawPool& TextDraws();
}} // namespace
