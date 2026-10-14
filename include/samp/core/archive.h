#pragma once
// CArchive over samp.saa. Observed: ctor 0x10064dd0, Open 0x10064f20 (vtable+4),
// Close 0x10064890, vtable 0x1011bae0, object 0x935 bytes.
#include <string>
#include <cstdint>
#include <windows.h>

namespace samp { namespace core {

class CArchive {
public:
  static constexpr size_t kObjectSize = 0x935;
  static constexpr uint32_t kAcceptMask  = 0x83433u;
  static constexpr uint32_t kAcceptBits  = 0x200000u;
  CArchive();
  ~CArchive();
  // Opens archive file (default "samp.saa" resolved next to DLL).
  // Returns true when XOR check (hdr0 ^ hdr1) passes masks above.
  bool Open(const char* path);
  void Close();
  bool IsOpen() const { return openFlag_ != 0; }
  // Behavior-equivalent read: virtual-handle aware wrapper lives in hooks_detour.
  bool ReadAt(uint32_t virtualOffset, void* out, uint32_t size);
  uint32_t EntryCap() const { return entryCap_; } // +0x119, observed 0x78
private:
  void*    vtbl_ = nullptr;
  uint8_t  base_[0x13] = {};
  char     openFlag_ = 0;      // +0x13
  void*    file_ = nullptr;    // +0x14 (CFile*)
  uint8_t  nameBuf_[0x100] = {};
  uint32_t hdr0_ = 0;          // +0x111
  uint32_t hdr1_ = 0;          // +0x115
  uint32_t entryCap_ = 0x78;   // +0x119
  void*    t1_ = nullptr;      // +0x91d-ish tables (opaque, TODO(rea))
  void*    t2_ = nullptr;
  void*    t3_ = nullptr;
  uint32_t f3_ = 0;            // +0x931
};

// Resolves "samp.saa" next to current module (GetModuleFileNameA + strip + append).
std::string ArchivePathNextToModule(HMODULE h);

}} // namespace
