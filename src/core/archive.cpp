// CArchive. Observed: ctor 0x10064dd0 (entryCap 0x78 @+0x119), Open 0x10064f20
// (2 virtual reads + XOR hdr check mask 0x83433/bits 0x200000), Close 0x10064890.
#include "samp/core/archive.h"
#include "samp/util/string_utils.h"
#include <windows.h>
#include <cstdio>

namespace samp { namespace core {

CArchive::CArchive() : entryCap_(0x78), f3_(0x100) {}
CArchive::~CArchive() { Close(); }

std::string ArchivePathNextToModule(HMODULE h) {
  char path[MAX_PATH] = {};
  GetModuleFileNameA(h, path, MAX_PATH);
  std::string s = path;
  auto pos = s.find_last_of("\\/");
  std::string dir = (pos == std::string::npos) ? "" : s.substr(0, pos + 1);
  return dir + "samp.saa";
}

bool CArchive::Open(const char* path) {
  FILE* f = nullptr;
  fopen_s(&f, path, "rb"); // original builds "rb" via FOpen_CRT(1,0)->__fsopen
  if (!f) return false;
  // Behavior-equivalent header check: read 8 bytes into hdr0/hdr1, XOR, mask.
  uint32_t a = 0, b = 0;
  size_t n1 = fread(&a, 1, 4, f);
  size_t n2 = fread(&b, 1, 4, f);
  fclose(f);
  if (n1 != 4 || n2 != 4) return false;
  hdr0_ = a; hdr1_ = b;
  uint32_t v = a ^ b;
  if ((v & 0xFFFFFu) != kAcceptMask) return false;
  if ((v & 0x700000u) != kAcceptBits) return false;
  file_ = (void*)0x1; // marker: open
  openFlag_ = 1;
  return true;
}

void CArchive::Close() {
  openFlag_ = 0;
  file_ = nullptr;
}

bool CArchive::ReadAt(uint32_t off, void* out, uint32_t size) {
  (void)off; (void)out; (void)size;
  if (!IsOpen()) return false;
  // TODO(rea): directory tables at +0x91d-ish + XOR key schedule unreversed.
  return false;
}

}} // namespace
