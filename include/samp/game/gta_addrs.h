#pragma once
// GTA:SA 1.0 x86 memory map. Ported from samp-source client/game/address.h
// (verified by grep) + globals observed in game.cpp/hooks.cpp/patches.cpp.
// These are ABSOLUTE addresses valid ONLY in the 32-bit GTA:SA process.
// On x64 (smoke-test) builds they are inert constants — NEVER dereference
// except under `#ifndef _WIN64` after the version gate passes.
// REA cross-check: DAT_00C8D4C0/00C97C1C/00BA6831/00747483/007F9D50/90
// all observed in samp.dll 0.3.DL-R1 (see docs/REVERSE_NOTES.md).
#include <cstdint>

namespace samp { namespace gta {

// --- Core (address.h:14-53) ---
inline constexpr uint32_t kHwnd            = 0x00C97C1Cu; // ADDR_HWND
inline constexpr uint32_t kD3DDevicePtr    = 0x00C97C28u; // ADDR_ID3D9DEVICE (COM-replaced by hook)
inline constexpr uint32_t kD3DPtr          = 0x00C97C20u; // ADDR_ID3D9
inline constexpr uint32_t kKeyStates       = 0x00B73458u;
inline constexpr uint32_t kMenu            = 0x00BA67A4u;
inline constexpr uint32_t kMenu2           = 0x00BA67A5u;
inline constexpr uint32_t kMenu3           = 0x00BA67A6u;
inline constexpr uint32_t kStartGame       = 0x00BA677Bu;
inline constexpr uint32_t kGameStarted     = 0x00BA6831u; // 1 = started
inline constexpr uint32_t kEntryState      = 0x00C8D4C0u; // wait ==7, pre=5, in-game=8
inline constexpr uint32_t kBypassVidsUS    = 0x00747483u; // 0x89 = USA10
inline constexpr uint32_t kBypassVidsEU    = 0x007474D3u; // 0xC8 = EU10
inline constexpr uint32_t kRwGetD3D_US     = 0x007F9D50u;
inline constexpr uint32_t kRwGetD3D_EU     = 0x007F9D90u;
inline constexpr uint32_t kEnableHud       = 0x00BA6769u;
inline constexpr uint32_t kRender2DStuff   = 0x0053E230u;
inline constexpr uint32_t kRender2DStorage = 0x0053E22Cu;
inline constexpr uint32_t kIdFromActor     = 0x00442D30u;
inline constexpr uint32_t kActorFromId     = 0x00404910u;
inline constexpr uint32_t kPedTable        = 0x00B74490u;
inline constexpr uint32_t kIdFromVehicle   = 0x0042C4B0u;
inline constexpr uint32_t kVehicleFromId   = 0x004048E0u;
inline constexpr uint32_t kVehicleTable    = 0x00B74494u;
inline constexpr uint32_t kObjectTable     = 0x0094DBE0u;
inline constexpr uint32_t kObjectFromId    = 0x00451C30u;
inline constexpr uint32_t kCamera          = 0x00B6F99Cu;
inline constexpr uint32_t kPlaceableVtbl   = 0x00863C40u;

// --- Extra globals used by hooks/game (file:line in samp-source) ---
inline constexpr uint32_t kCamModeByte     = 0x00B6F1A8u; // hooks.cpp:51
inline constexpr uint32_t kCurPlayerIdx    = 0x00B7CD74u; // hooks.cpp:52
inline constexpr uint32_t kCamModeWord     = 0x00B6F858u; // hooks.cpp:53
inline constexpr uint32_t kScreenW         = 0x00C17044u; // game.h:160
inline constexpr uint32_t kScreenH         = 0x00C17048u;
inline constexpr uint32_t kHudScaleX       = 0x00859520u; // game.h:162
inline constexpr uint32_t kHudScaleY       = 0x00859524u;
inline constexpr uint32_t kRadarPosX       = 0x00866B70u; // patches.cpp:446
inline constexpr uint32_t kStreamingMem    = 0x005B8E6Au; // patches.cpp:291
inline constexpr uint32_t kWantedLevel     = 0x0058DB60u;
inline constexpr uint32_t kWeatherCur      = 0x00C81320u; // game.cpp:441
inline constexpr uint32_t kWeatherLock     = 0x00C8131Cu;
inline constexpr uint32_t kLocalMoney      = 0x00B7CE50u; // game.cpp:862
inline constexpr uint32_t kGameSpeed       = 0x00B7CB64u;
inline constexpr uint32_t kGravity         = 0x00863984u; // game.cpp:967
inline constexpr uint32_t kPickupPool      = 0x009788C0u; // hooks.cpp:1209 (stride 0x20)
inline constexpr uint32_t kLoadingActive   = 0x00BAB318u; // patches.cpp:745
inline constexpr uint32_t kRadioVolume     = 0x00B5FCC8u; // game.cpp:1102

// D3D9 vtable slots (COM order: IUnknown 0-2; Reset=16, Present=17).
inline constexpr int kD3DResetSlot   = 16;
inline constexpr int kD3DPresentSlot = 17;

// Helper: read/write only meaningful on x86 GTA process.
template <typename T>
inline T* X86Ptr(uint32_t absAddr) {
  return reinterpret_cast<T*>(static_cast<uintptr_t>(absAddr));
}

}} // namespace
