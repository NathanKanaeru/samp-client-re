#pragma once
// DL-R1 custom model requests. Observed strings (REA, samp.dll):
// `%s/0x%X.dff`, `%s\0x%X.dff`, `%s/*.dff`, `[warning] Could not validate…`,
// `%s/0x%X.txd`, `Too many textures…`, served via FUN_1000BB10 through
// FUN_100b5900. SAMP.img (47 MB) + custom.img hold the data; names are built
// at runtime (no literal in binary). States below mirror a download/validate
// queue; wire download itself is TODO(rea).
#include <cstdint>
#include <cstdio>
#include <string>

namespace samp { namespace game {

enum class ModelReqState : uint8_t { Idle, Requested, Downloading, Validating, Ready, Failed };

struct ModelRequest {
  uint32_t id = 0; // hex key in %s/0x%X.dff pattern
  ModelReqState dff = ModelReqState::Idle;
  ModelReqState txd = ModelReqState::Idle;
};

inline std::string DffName(const std::string& dir, uint32_t id) {
  char b[64];
  snprintf(b, sizeof b, "%s/0x%X.dff", dir.c_str(), id);
  return b;
}
inline std::string TxdName(const std::string& dir, uint32_t id) {
  char b[64];
  snprintf(b, sizeof b, "%s/0x%X.txd", dir.c_str(), id);
  return b;
}

class ModelQueue {
public:
  static constexpr int kMax = 64;
  bool Request(uint32_t id);            // -> Requested (false if dup/full)
  bool MarkDownloading(uint32_t id);
  bool MarkReady(uint32_t id);          // validated
  bool MarkFailed(uint32_t id);         // "Could not validate…"
  const ModelRequest* Find(uint32_t id) const;
  int Count() const { return count_; }
private:
  ModelRequest slots_[kMax];
  int count_ = 0;
};

}} // namespace
