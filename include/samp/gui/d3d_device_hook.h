#pragma once
// IDirect3DDevice9 hook without the DX SDK. Technique ported from libsamp
// d3dhook.* (MIT): COM-object replacement at GTA's device pointer
// (gta::kD3DDevicePtr 0x00C97C28) — only Present (slot 17) / Reset (slot 16)
// are overridden; everything else forwards to the original vtable.
// ABI note: real D3D9 methods are __thiscall on x86 (this in ECX), so the
// overrides below are __thiscall too (no-op on x64, supported by MinGW on
// x86; MSVC x86 builds need this file compiled with a thiscall-capable
// compiler — see note in d3d_device_hook.cpp). No inline ASM per repo rules.
#include <cstdint>
#include "samp/game/gta_addrs.h"

namespace samp { namespace gui {

// __thiscall: `self` arrives in ECX on x86, as first arg on x64.
using PresentFn = long(__thiscall*)(void* self, const void* src, const void* dst,
                                    void* wndOverride, const void* dirtyRegion);
using ResetFn = long(__thiscall*)(void* self, void* presentParams);

struct D3DDeviceHook {
  void* realDevice = nullptr;   // borrowed GTA device
  void** tableCopy = nullptr;   // our vtable copy (methodCount entries)
  int methodCount = 0;
  PresentFn origPresent = nullptr;
  ResetFn origReset = nullptr;
  // Callbacks wired by the GUI layer (may be null).
  void (*onFrame)() = nullptr;  // called inside Present override
  void (*onLost)() = nullptr;   // called inside Reset override (pre-reset)
  void (*onReset)() = nullptr;  // called in Reset when result == 0 (D3D_OK)
};

// Copies `methodCount` vtable slots from device and installs overrides.
// Returns nullptr on null device / count < 18.
D3DDeviceHook* HookDevice(void* device, int methodCount = 119);
// Frees the copy (game memory is never patched by this helper).
void UnhookDevice(D3DDeviceHook* h);
// The vtable pointer a hooked client should expose to the engine.
inline void** HookVTable(D3DDeviceHook* h) { return h ? h->tableCopy : nullptr; }

}} // namespace
