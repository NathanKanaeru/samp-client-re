#pragma once
// Audio (BASS). Observed: 17 imports, FUN_10065f00 init
// (BASS_Init(-1,0xAC44), SetConfigPtr "SA-MP/0.3"), FUN_100663e0 play request,
// thread 0x10066180 (StreamCreateURL flags 0x940000 + ChannelPlay + tags +
// SetSync), FUN_10066040 tag parse. 3D setters imported, callers unresolved.
#include <string>
namespace samp { namespace audio {
bool Init(); // FUN_10065f00
bool PlayStream(const std::string& url, float x, float y, float z, bool use3d); // FUN_100663e0
void Stop();
void AudioThread(); // 0x10066180
}} // namespace
