#pragma once
// Audio stream. Observed: BASS 17 imports, FUN_10065f00 init,
// FUN_100663e0 request, thread 0x10066180, FUN_10066040 tag parse.
// Reference (samp-source audiostream.*): BASS_Init(-1,44100),
// NET_AGENT "SA-MP/0.3", flags BLOCK|STATUS|AUTOFREE, Meta+Ogg sync,
// Process() every Present: radio duck, >=100ms volume tick
// (0 if menu/no-focus; linear 1-dist/maxDist * radioVol*8000 when in range),
// icy-name + title label at bottom-left.
#include <cstdint>
#include <string>

namespace samp { namespace audio {

inline constexpr unsigned kBassRate = 44100;
inline constexpr const char* kNetAgent = "SA-MP/0.3";
inline constexpr unsigned kVolumeTickMs = 100;

// Pure volume law (tested): radioVol 0..1 scaled to BASS 0..8000.
inline int VolumeFor(float dist, float maxDist, float radioVol) {
  if (dist < 0 || maxDist <= 0) return (int)(radioVol * 8000.0f);
  if (dist >= maxDist) return 0;
  return (int)((1.0f - dist / maxDist) * radioVol * 8000.0f);
}

// Parses `StreamTitle='...';` (ICY meta) or `artist=`/`title=` (OGG).
// Returns title (may be empty). Pure + tested.
std::string ParseMetaTitle(const std::string& tag, const std::string& tagName);

struct StreamState {
  std::string url;
  float x = 0, y = 0, z = 0, maxDist = 0; // maxDist<0 = 2D (no attenuate)
  std::string title;
  std::string label; // "icy-name + title" rendered bottom-left
  bool playing = false;
  uint32_t lastTick = 0;
};

bool Init(); // FUN_10065f00 (dynamic-load BASS, stub-safe without dll)
bool PlayStream(const std::string& url, float x, float y, float z, bool use3d);
void Stop();
void AudioThread(); // 0x10066180
// Per-Present tick (testable): returns volume to apply, updates label.
// menuOpen/noFocus force 0; radioDuck silences GTA radio (callback).
struct TickCtx {
  bool menuOpen = false;
  bool hasFocus = true;
  float radioVol = 1.0f; // 0..1 from CGame::GetRadioVolume
  uint32_t nowMs = 0;
  void (*duckRadio)(bool on) = nullptr;
};
int ProcessTick(StreamState& st, const TickCtx& ctx);

}} // namespace
