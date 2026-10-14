#pragma once
// BitStream. Observed: 0x1001f300 SetData, 0x1001f1f0 Reset, 0x1001f390 Free,
// 0x1001f470 ReadBits, 0x1001f960 ReadBytesAligned, 0x1001faa0 WriteBits,
// 0x1001fb50 WriteBytes, 0x1001f630 EnsureCapacity.
#include <cstdint>
#include <cstddef>
#include <vector>

namespace samp { namespace net {

class BitStream {
public:
  BitStream();
  BitStream(const void* data, size_t len, bool copy);
  ~BitStream();
  void SetData(const void* data, size_t len, bool copy);
  void Reset();
  bool ReadBits(void* out, size_t nBits, bool alignRight = true);
  bool ReadBytes(void* out, size_t n);
  bool ReadU8(uint8_t& v)  { return ReadBytes(&v, 1); }
  bool ReadU16(uint16_t& v);
  bool ReadU32(uint32_t& v);
  void WriteBits(const void* src, size_t nBits);
  void WriteBytes(const void* src, size_t n);
  void WriteU8(uint8_t v)  { WriteBytes(&v, 1); }
  void WriteU16(uint16_t v);
  void WriteU32(uint32_t v);
  void WriteStr(const char* s); // raw bytes, no length prefix (matches join packet)
  // RakNet-compatible compressed unit float (verified vs Knogle/RakNet
  // BitStream.h:907-915,1124-1129): clamp [-1,1], u16 (v+1)*32767.5.
  void WriteCompressedUnit(float v);
  bool ReadCompressedUnit(float& v);
  // open.mp-style compressed vector: magnitude float, then 3 units only when
  // magnitude > 1e-5 (libsamp openmp_compressed_vector_compat rule).
  void WriteCompVector(float mag, float x, float y, float z);
  bool ReadCompVector(float& mag, float out[3]);
  size_t BitsUsed() const { return bitsUsed_; }
  size_t ReadOffset() const { return readOff_; }
  const uint8_t* Data() const { return data_; }
private:
  bool EnsureCapacity(size_t extraBits);
  uint8_t  inlineBuf_[0x100] = {};
  uint8_t* data_ = inlineBuf_;
  size_t   bitsUsed_ = 0;   // this[0]
  size_t   capacityBits_ = 0x800; // 0x100 bytes inline
  size_t   readOff_ = 0;    // this[2]
  bool     owned_ = false;
};

}} // namespace
