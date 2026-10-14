// Audio. Observed: BASS 17 imports, FUN_10065f00 init, FUN_100663e0 request,
// thread 0x10066180 (StreamCreateURL 0x940000 + Play + tags + SetSync).
// Reference logic (audiostream.cpp:109-223) implemented in ProcessTick /
// ParseMetaTitle (pure + tested); BASS/GTA calls stay dynamic and stub-safe.
#include "samp/audio/audio.h"
#include <windows.h>
#include <cmath>
#include <cstring>

namespace samp { namespace audio {

static HMODULE g_bass = nullptr;
static StreamState g_stream;

static void* Sym(const char* n) { return g_bass ? (void*)GetProcAddress(g_bass, n) : nullptr; }

bool Init() { // FUN_10065f00
  g_bass = LoadLibraryA("bass.dll");
  if (!g_bass) return false; // keep client usable without audio dll
  using InitFn = int(*)(int, unsigned, unsigned, void*, void*);
  // Reference uses 44100 Hz (audiostream.cpp:29-55); observed binary used 0xAC44.
  if (auto* f = (InitFn)Sym("BASS_Init")) f(-1, kBassRate, 0, 0, 0);
  return true;
}

static DWORD WINAPI ThreadFn(LPVOID) { AudioThread(); return 0; }

bool PlayStream(const std::string& url, float x, float y, float z, bool use3d) { // FUN_100663e0
  g_stream.url = url;
  g_stream.x = x; g_stream.y = y; g_stream.z = z;
  g_stream.maxDist = use3d ? 60.0f : -1.0f;
  g_stream.playing = true;
  CreateThread(nullptr, 0, ThreadFn, nullptr, 0, nullptr);
  return true;
}
void Stop() { g_stream.playing = false; }

void AudioThread() { // 0x10066180 abridged
  (void)g_stream.url; // StreamCreateURL + ChannelPlay + GetTags/SetSync + 3D poll
}

std::string ParseMetaTitle(const std::string& tag, const std::string& tagName) {
  // ICY: StreamTitle='...';
  const std::string key = "StreamTitle='";
  auto p = tag.find(key);
  if (p != std::string::npos) {
    auto e = tag.find('\'', p + key.size());
    if (e != std::string::npos) return tag.substr(p + key.size(), e - p - key.size());
  }
  // OGG: artist= / title= (tagName selects which key was delivered)
  for (const char* k : {"artist=", "title="}) {
    if (tagName == k || tag.rfind(k, 0) == 0) {
      auto v = tag.find(k);
      if (v != std::string::npos) return tag.substr(v + strlen(k));
    }
  }
  return "";
}

int ProcessTick(StreamState& st, const TickCtx& ctx) {
  if (!st.playing) return 0;
  if (ctx.duckRadio) ctx.duckRadio(true); // silence GTA radio
  if (ctx.menuOpen || !ctx.hasFocus) return 0;
  if (ctx.nowMs - st.lastTick < kVolumeTickMs) return -1; // no update due
  st.lastTick = ctx.nowMs;
  float dx = st.x, dy = st.y, dz = st.z;
  float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
  if (st.maxDist < 0) return (int)(ctx.radioVol * 8000.0f); // 2D
  return VolumeFor(dist, st.maxDist, ctx.radioVol);
}

}} // namespace
