#pragma once
// Network limits. Ported from samp-source sdk/shared.h (verified by grep).
// NOTE on discrepancy: samp.dll 0.3.DL-R1's debug overlay (FUN_10004710)
// checks caps 2000/1004/2101/1000, while shared.h declares net slots
// 200/700/255/1000. Net_* here = authoritative network slots (shared.h);
// overlay caps are GTA-side pool guards, documented in game/pools.h.
#include <cstdint>

namespace samp { namespace net {

inline constexpr const char* kNetVersionString = "1.0-alpha"; // SAMP_VERSION shared.h:5
inline constexpr uint32_t kNetGameVersion = 4242;             // NETGAME_VERSION shared.h:6
inline constexpr uint32_t kGpciKey        = 1001;             // shared.h:7
inline constexpr uint8_t  kModVersion     = 0x01;             // client/mod.h:15

inline constexpr int kMaxPlayers        = 200;  // shared.h:12
inline constexpr int kMaxVehicles       = 700;  // shared.h:13
inline constexpr int kMaxObjects        = 255;  // shared.h:14
inline constexpr int kMaxMenus          = 128;  // shared.h:15
inline constexpr int kMaxTextDraws      = 2048; // shared.h:16
inline constexpr int kMaxPlayerTextDraw = 256;  // shared.h:17
inline constexpr int kMaxTextDrawSlots  = 2304; // 2048+256
inline constexpr int kMaxGangZones      = 1024; // shared.h:18
inline constexpr int kMaxPickups        = 400;  // shared.h:24
inline constexpr int kMaxActors         = 1000; // shared.h:27
inline constexpr int kMaxLabelGlobal    = 1024; // shared.h:31
inline constexpr int kMaxLabelPlayer    = 1024; // shared.h:32
inline constexpr int kMaxLabels         = 2048;
inline constexpr int kMaxSpawns         = 500;  // shared.h:20
inline constexpr int kMaxPlayerName     = 24;
inline constexpr int kMaxTextDrawLine   = 800;  // shared.h:25
inline constexpr int kMaxChatBubbleText = 144;  // shared.h:29
inline constexpr int kMaxLabelText      = 2048; // shared.h:30
inline constexpr int kMaxDialogCaption  = 64;   // shared.h:33

}} // namespace
