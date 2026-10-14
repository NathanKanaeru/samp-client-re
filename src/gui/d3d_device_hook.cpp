// D3D vtable-copy hook. See d3d_device_hook.h for provenance.
// MSVC-x86 note: free-function __thiscall below needs MinGW (or clang-cl);
// with MSVC cl, compile this TU with /Gz-disabled default and provide tiny
// naked trampolines instead (TODO(rea) if an MSVC-x86 build is ever required).
#include "samp/gui/d3d_device_hook.h"
#include <windows.h>
#include <cstring>
#include <new>

namespace samp { namespace gui {

namespace {
D3DDeviceHook* g_hooks[8] = {};
} // namespace

static D3DDeviceHook* FindHookByDevice(void* dev) {
  for (auto* h : g_hooks)
    if (h && h->realDevice == dev) return h;
  return nullptr;
}

static long __thiscall PresentOverride(void* self, const void* src, const void* dst,
                                       void* wnd, const void* dirty) {
  D3DDeviceHook* h = FindHookByDevice(self);
  if (h && h->onFrame) h->onFrame();
  if (!h || !h->origPresent) return -1; // E_FAIL-ish without DX headers
  return h->origPresent(self, src, dst, wnd, dirty);
}

static long __thiscall ResetOverride(void* self, void* params) {
  D3DDeviceHook* h = FindHookByDevice(self);
  if (h && h->onLost) h->onLost();
  if (!h || !h->origReset) return -1;
  long hr = h->origReset(self, params);
  if (hr == 0 && h->onReset) h->onReset(); // D3D_OK
  return hr;
}

D3DDeviceHook* HookDevice(void* device, int methodCount) {
  if (!device || methodCount < 18) return nullptr;
  void** orig = *(void***)device;
  if (!orig) return nullptr;
  auto* h = new (std::nothrow) D3DDeviceHook();
  if (!h) return nullptr;
  h->tableCopy = new (std::nothrow) void*[(size_t)methodCount];
  if (!h->tableCopy) { delete h; return nullptr; }
  memcpy(h->tableCopy, orig, sizeof(void*) * (size_t)methodCount);
  h->realDevice = device;
  h->methodCount = methodCount;
  h->origPresent = (PresentFn)orig[gta::kD3DPresentSlot];
  h->origReset = (ResetFn)orig[gta::kD3DResetSlot];
  h->tableCopy[gta::kD3DPresentSlot] = (void*)PresentOverride;
  h->tableCopy[gta::kD3DResetSlot] = (void*)ResetOverride;
  for (auto& s : g_hooks)
    if (!s) { s = h; break; }
  return h;
}

void UnhookDevice(D3DDeviceHook* h) {
  if (!h) return;
  for (auto& s : g_hooks)
    if (s == h) s = nullptr;
  delete[] h->tableCopy;
  delete h;
}

}} // namespace
