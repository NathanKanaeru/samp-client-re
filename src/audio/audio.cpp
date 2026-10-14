// Audio. Observed: BASS 17 imports, FUN_10065f00 init, FUN_100663e0 request,
// thread 0x10066180 (StreamCreateURL 0x940000 + Play + tags + SetSync).
// Build without BASS SDK: dynamic load, stub when absent.
#include "samp/audio/audio.h"
#include <windows.h>
#include <string>

namespace samp { namespace audio {

static HMODULE g_bass = nullptr;
static std::string g_url;

static void* Sym(const char* n) { return g_bass ? (void*)GetProcAddress(g_bass, n) : nullptr; }

bool Init() { // FUN_10065f00
  g_bass = LoadLibraryA("bass.dll");
  if (!g_bass) return false; // keep client usable without audio dll
  using InitFn = int(*)(int, unsigned, unsigned, void*, void*);
  if (auto* f = (InitFn)Sym("BASS_Init")) f(-1, 0xAC44, 0, 0, 0);
  return true;
}

static DWORD WINAPI ThreadFn(LPVOID) { AudioThread(); return 0; }

bool PlayStream(const std::string& url, float, float, float, bool) { // FUN_100663e0
  g_url = url;
  CreateThread(nullptr, 0, ThreadFn, nullptr, 0, nullptr);
  return true;
}
void Stop() {}
void AudioThread() { // 0x10066180 abridged
  (void)g_url; // StreamCreateURL + ChannelPlay + GetTags/SetSync + 3D poll
}

}} // namespace
