// BitStream. Observed layout: inline buf +0x11 cap 0x100, SetData copies when
// len>=0x100 else inline; Reset sets cap 0x800 bits; readers/writers bit-exact.
#include "samp/net/bitstream.h"
#include "samp/net/compressed_vector.h"
#include <cstring>
#include <cstdlib>

namespace samp { namespace net {

BitStream::BitStream() { Reset(); }
BitStream::BitStream(const void* d, size_t n, bool c) { SetData(d, n, c); }
BitStream::~BitStream() {
  if (owned_ && data_ != inlineBuf_) free(data_); // FUN_1001f390
}

void BitStream::SetData(const void* data, size_t len, bool copy) { // FUN_1001f300
  Reset();
  bitsUsed_ = len * 8;
  readOff_ = 0;
  if (!copy) { data_ = (uint8_t*)data; owned_ = false; capacityBits_ = bitsUsed_; return; }
  if (len < 0x100) { memcpy(inlineBuf_, data, len); data_ = inlineBuf_; owned_ = false; capacityBits_ = 0x800; }
  else { data_ = (uint8_t*)malloc(len); memcpy(data_, data, len); owned_ = true; capacityBits_ = len * 8; }
}

void BitStream::Reset() { // FUN_1001f1f0
  if (owned_ && data_ != inlineBuf_) free(data_);
  data_ = inlineBuf_; bitsUsed_ = 0; capacityBits_ = 0x800; readOff_ = 0; owned_ = false;
}

bool BitStream::EnsureCapacity(size_t extraBits) { // FUN_1001f630
  size_t need = bitsUsed_ + extraBits;
  if (need <= capacityBits_) return true;
  size_t ncap = capacityBits_ ? capacityBits_ * 2 : 0x800;
  while (ncap < need) ncap *= 2;
  uint8_t* nd = (uint8_t*)malloc(ncap / 8);
  if (!nd) return false;
  memcpy(nd, data_, bitsUsed_ / 8 + 1);
  if (owned_ && data_ != inlineBuf_) free(data_);
  data_ = nd; owned_ = true; capacityBits_ = ncap;
  return true;
}

bool BitStream::ReadBits(void* out, size_t nBits, bool alignRight) { // FUN_1001f470
  if (readOff_ + nBits > bitsUsed_) return false;
  uint32_t v = 0;
  for (size_t i = 0; i < nBits; ++i) {
    size_t b = readOff_ + i;
    uint8_t bit = (data_[b / 8] >> (7 - (b % 8))) & 1;
    v = (v << 1) | bit;
  }
  readOff_ += nBits;
  if (!alignRight) v <<= (32 - (uint32_t)nBits);
  size_t nBytes = (nBits + 7) / 8;
  memset(out, 0, nBytes);
  for (size_t i = 0; i < nBytes; ++i)
    ((uint8_t*)out)[nBytes - 1 - i] = (uint8_t)(v >> (i * 8));
  return true;
}

bool BitStream::ReadBytes(void* out, size_t n) { // FUN_1001f960 fast-path
  if (readOff_ % 8 == 0) {
    size_t off = readOff_ / 8;
    if ((off + n) * 8 > bitsUsed_) return false;
    memcpy(out, data_ + off, n);
    readOff_ += n * 8;
    return true;
  }
  return ReadBits(out, n * 8, true);
}

bool BitStream::ReadU16(uint16_t& v) { uint8_t b[2]; if (!ReadBytes(b, 2)) return false; v = b[0] | (uint16_t(b[1]) << 8); return true; }
bool BitStream::ReadU32(uint32_t& v) { uint8_t b[4]; if (!ReadBytes(b, 4)) return false; v = b[0] | (uint32_t(b[1]) << 8) | (uint32_t(b[2]) << 16) | (uint32_t(b[3]) << 24); return true; }

void BitStream::WriteBits(const void* src, size_t nBits) { // FUN_1001faa0
  if (!EnsureCapacity(nBits)) return;
  const uint8_t* s = (const uint8_t*)src;
  for (size_t i = 0; i < nBits; ++i) {
    size_t sb = nBits - 1 - i;
    uint8_t bit = (s[sb / 8] >> (sb % 8)) & 1;
    size_t db = bitsUsed_ + i;
    if (bit) data_[db / 8] |= (uint8_t)(1 << (7 - (db % 8)));
    else data_[db / 8] &= (uint8_t)~(1 << (7 - (db % 8)));
  }
  bitsUsed_ += nBits;
}

void BitStream::WriteBytes(const void* src, size_t n) { // FUN_1001fb50
  if (bitsUsed_ % 8 == 0) {
    if (!EnsureCapacity(n * 8)) return;
    memcpy(data_ + bitsUsed_ / 8, src, n);
    bitsUsed_ += n * 8;
  } else WriteBits(src, n * 8);
}

void BitStream::WriteU16(uint16_t v) { uint8_t b[2] = {(uint8_t)v, (uint8_t)(v >> 8)}; WriteBytes(b, 2); }
void BitStream::WriteU32(uint32_t v) { uint8_t b[4] = {(uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24)}; WriteBytes(b, 4); }
void BitStream::WriteStr(const char* s) { WriteBytes(s, strlen(s)); }

void BitStream::WriteCompressedUnit(float v) {
  if (v < -1.0f) v = -1.0f;
  if (v > 1.0f) v = 1.0f;
  WriteU16((uint16_t)((v + 1.0f) * 32767.5f));
}

bool BitStream::ReadCompressedUnit(float& v) {
  uint16_t q = 0;
  if (!ReadU16(q)) return false;
  v = (float)q / 32767.5f - 1.0f;
  return true;
}

void BitStream::WriteCompVector(float mag, float x, float y, float z) {
  WriteBytes(&mag, 4);
  if (mag > net::kVecMagnitudeEpsilon) {
    WriteCompressedUnit(x);
    WriteCompressedUnit(y);
    WriteCompressedUnit(z);
  }
}

bool BitStream::ReadCompVector(float& mag, float out[3]) {
  if (!ReadBytes(&mag, 4)) return false;
  if (mag > net::kVecMagnitudeEpsilon)
    return ReadCompressedUnit(out[0]) && ReadCompressedUnit(out[1]) && ReadCompressedUnit(out[2]);
  out[0] = out[1] = out[2] = 0.0f;
  return true;
}

}} // namespace
